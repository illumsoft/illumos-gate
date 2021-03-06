/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"@(#)smb_nic.c	1.6	08/07/24 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <libintl.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <synch.h>
#include <stropts.h>
#include <errno.h>
#include <pthread.h>

#include <inet/ip.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/route.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/systeminfo.h>

#include <smbsrv/libsmb.h>

#define	SMB_NIC_DB_NAME		"/var/smb/smbhosts.db"
#define	SMB_NIC_DB_TIMEOUT	3000		/* in millisecond */
#define	SMB_NIC_DB_VERMAJOR	1
#define	SMB_NIC_DB_VERMINOR	0
#define	SMB_NIC_DB_MAGIC	0x484F5354	/* HOST */

#define	SMB_NIC_DB_ORD		1		/* open read-only */
#define	SMB_NIC_DB_ORW		2		/* open read/write */

#define	SMB_NIC_DB_SQL \
	"CREATE TABLE db_info ("				\
	"	ver_major INTEGER,"				\
	"	ver_minor INTEGER,"				\
	"	magic     INTEGER"				\
	");"							\
	""							\
	"CREATE TABLE hosts ("					\
	"	hostname  TEXT PRIMARY KEY,"			\
	"	comment   TEXT,"				\
	"	ifnames   TEXT"					\
	");"

#define	SMB_NIC_HTBL_NCOL	3
#define	SMB_NIC_HTBL_HOST	0
#define	SMB_NIC_HTBL_CMNT	1
#define	SMB_NIC_HTBL_IFS	2

#define	NULL_MSGCHK(msg)	((msg) ? (msg) : "NULL")

#define	SMB_NIC_MAXIFS		256

typedef struct smb_hostifs {
	list_node_t if_lnd;
	char if_host[MAXHOSTNAMELEN];
	char if_cmnt[SMB_PI_MAX_COMMENT];
	char *if_names[SMB_NIC_MAXIFS];
	int if_num;
} smb_hostifs_t;

typedef struct smb_hosts {
	list_t h_list;
	int h_num;
	int h_ifnum;
} smb_hosts_t;

typedef struct {
	smb_nic_t	*nl_nics;
	int		nl_cnt;		/* number of smb_nic_t structures */
	int		nl_hcnt;	/* number of host names */
	long		nl_seqnum;	/* a random sequence number */
	rwlock_t	nl_rwl;
} smb_niclist_t;

static int smb_nic_list_create(void);
static void smb_nic_list_destroy(void);

static int smb_nic_hlist_create(smb_hosts_t *);
static void smb_nic_hlist_destroy(smb_hosts_t *);
static int smb_nic_hlist_dbget(smb_hosts_t *);
static int smb_nic_hlist_sysget(smb_hosts_t *);

static void smb_nic_iflist_destroy(smb_hostifs_t *);
static smb_hostifs_t *smb_nic_iflist_decode(const char **);

static int smb_nic_dbcreate(void);
static sqlite *smb_nic_dbopen(int);
static void smb_nic_dbclose(sqlite *);
static boolean_t smb_nic_dbexists(void);
static boolean_t smb_nic_dbvalidate(void);
static int smb_nic_dbaddhost(const char *, const char *, char *);
static int smb_nic_dbdelhost(const char *);
static int smb_nic_dbsetinfo(sqlite *);

static int smb_nic_getinfo(char *, smb_nic_t *);

/* This is the list we will monitor */
static smb_niclist_t smb_niclist;

/*
 * smb_nic_init
 *
 * Initializes the interface list.
 */
int
smb_nic_init(void)
{
	int rc;

	(void) rw_wrlock(&smb_niclist.nl_rwl);
	smb_nic_list_destroy();
	rc = smb_nic_list_create();
	(void) rw_unlock(&smb_niclist.nl_rwl);

	return (rc);
}

/*
 * smb_nic_fini
 *
 * Destroys the interface list.
 */
void
smb_nic_fini(void)
{
	(void) rw_wrlock(&smb_niclist.nl_rwl);
	smb_nic_list_destroy();
	(void) rw_unlock(&smb_niclist.nl_rwl);
}

/*
 * smb_nic_getnum
 *
 * Gets the number of interfaces for the specified host.
 * if host is NULL then total number of interfaces
 * is returned. It's assumed that given name is a NetBIOS
 * encoded name.
 */
int
smb_nic_getnum(char *nb_hostname)
{
	int n = 0, i;

	(void) rw_rdlock(&smb_niclist.nl_rwl);

	if (nb_hostname != NULL) {
		for (i = 0; i < smb_niclist.nl_cnt; i++) {
			/* ignore the suffix */
			if (strncasecmp(smb_niclist.nl_nics[i].nic_nbname,
			    nb_hostname, NETBIOS_NAME_SZ - 1) == 0)
				n++;
		}
	} else {
		n = smb_niclist.nl_cnt;
	}

	(void) rw_unlock(&smb_niclist.nl_rwl);

	return (n);
}

/*
 * smb_nic_getfirst
 *
 * Returns the first NIC in the interface list and
 * initializes the given iterator. To get the rest of
 * NICs smb_nic_getnext() must be called.
 *
 * Returns 0 upon success and -1 if there's no interface
 * available or if 'ni' is NULL.
 */
int
smb_nic_getfirst(smb_niciter_t *ni)
{
	int rc = 0;

	if (ni == NULL)
		return (-1);

	(void) rw_rdlock(&smb_niclist.nl_rwl);

	if (smb_niclist.nl_cnt > 0) {
		ni->ni_nic = smb_niclist.nl_nics[0];
		ni->ni_cookie = 1;
		ni->ni_seqnum = smb_niclist.nl_seqnum;
	} else {
		rc = -1;
	}

	(void) rw_unlock(&smb_niclist.nl_rwl);

	return (rc);
}

/*
 * smb_nic_getnext
 *
 * Returns the next NIC information based on the passed
 * iterator (ni). The iterator must have previously been
 * initialized by calling smb_nic_getfirst().
 *
 * Returns 0 upon successfully finding the specified NIC.
 * Returns -1 if:
 * 	- the specified iterator is invalid
 * 	- reaches the end of the NIC list
 * 	- sequence number in the iterator is different from
 *	  the sequence number in the NIC list which means
 *	  the list has been changed between getfirst/getnext
 *	  calls.
 */
int
smb_nic_getnext(smb_niciter_t *ni)
{
	int rc = 0;

	if ((ni == NULL) || (ni->ni_cookie < 1))
		return (-1);

	(void) rw_rdlock(&smb_niclist.nl_rwl);

	if ((smb_niclist.nl_cnt > ni->ni_cookie) &&
	    (smb_niclist.nl_seqnum == ni->ni_seqnum)) {
		ni->ni_nic = smb_niclist.nl_nics[ni->ni_cookie];
		ni->ni_cookie++;
	} else {
		rc = -1;
	}

	(void) rw_unlock(&smb_niclist.nl_rwl);

	return (rc);
}

/*
 * smb_nic_exists
 *
 * Check to see if there's a NIC with the given IP address
 * in the list. Subnet mask will be applied when comparing the
 * IPs if the use_mask arg is true.
 */
boolean_t
smb_nic_exists(uint32_t ipaddr, boolean_t use_mask)
{
	smb_nic_t *cfg;
	uint32_t mask = 0xFFFFFFFF;
	int i;

	(void) rw_rdlock(&smb_niclist.nl_rwl);

	for (i = 0; i < smb_niclist.nl_cnt; i++) {
		cfg = &smb_niclist.nl_nics[i];
		if (use_mask)
			mask = cfg->nic_mask;

		if ((ipaddr & mask) == (cfg->nic_ip & mask)) {
			(void) rw_unlock(&smb_niclist.nl_rwl);
			return (B_TRUE);
		}
	}

	(void) rw_unlock(&smb_niclist.nl_rwl);

	return (B_FALSE);
}

/*
 * smb_nic_addhost
 *
 * Adds an association between the given host and the specified interface
 * list (if_names). This function can be called as many times as needed,
 * the associations will be stored in /var/smb/smbhosts.db, which is sqlite
 * database. If this file exists and it's not empty NIC list is generated
 * based on the information stored in this file.
 *
 * host: actual system's name (not Netbios name)
 * cmnt: an optional description for the CIFS server running on
 *       the specified host. Can be NULL.
 * if_num: number of interface names in if_names arg
 * if_names: array of interface names in string format
 *
 * Returns 0 upon success and -1 when fails
 */
int
smb_nic_addhost(const char *host, const char *cmnt,
    int if_num, const char **if_names)
{
	char *if_list;
	char *ifname;
	int buflen = 0;
	int rc, i;

	if ((host == NULL) || (if_num <= 0) || (if_names == NULL))
		return (-1);

	if (!smb_nic_dbexists() || !smb_nic_dbvalidate()) {
		if (smb_nic_dbcreate() != SQLITE_OK)
			return (-1);
	}

	ifname = (char *)if_names;
	for (i = 0; i < if_num; i++, ifname++) {
		if ((ifname == NULL) || (*ifname == '\0'))
			return (-1);
		buflen += strlen(ifname) + 1;
	}

	if ((if_list = malloc(buflen)) == NULL)
		return (-1);

	ifname = if_list;
	for (i = 0; i < if_num - 1; i++)
		ifname += snprintf(ifname, buflen, "%s,", if_names[i]);

	(void) snprintf(ifname, buflen, "%s", if_names[i]);

	rc = smb_nic_dbaddhost(host, cmnt, if_list);
	free(if_list);

	return ((rc == SQLITE_OK) ? 0 : -1);
}

/*
 * smb_nic_delhost
 *
 * Removes the stored interface association for the specified host
 */
int
smb_nic_delhost(const char *host)
{
	if ((host == NULL) || (*host == '\0'))
		return (-1);

	if (!smb_nic_dbexists())
		return (0);

	if (!smb_nic_dbvalidate()) {
		(void) unlink(SMB_NIC_DB_NAME);
		return (0);
	}

	if (smb_nic_dbdelhost(host) != SQLITE_OK)
		return (-1);

	return (0);
}

/*
 * smb_nic_list_create
 *
 * Creates a NIC list either based on /var/smb/smbhosts.db or
 * by getting the information from OS.
 *
 * Note that the caller of this function should grab the
 * list lock.
 */
static int
smb_nic_list_create(void)
{
	smb_hosts_t hlist;
	smb_hostifs_t *iflist;
	smb_nic_t *nc;
	char *ifname;
	char excludestr[MAX_EXCLUDE_LIST_LEN];
	ipaddr_t exclude[SMB_PI_MAX_NETWORKS];
	int nexclude;
	int i;

	if (smb_nic_hlist_create(&hlist) < 0)
		return (-1);

	smb_niclist.nl_cnt = 0;
	smb_niclist.nl_seqnum = random();
	smb_niclist.nl_hcnt = hlist.h_num;

	smb_niclist.nl_nics = calloc(hlist.h_ifnum, sizeof (smb_nic_t));
	if (smb_niclist.nl_nics == NULL) {
		smb_nic_hlist_destroy(&hlist);
		return (-1);
	}

	(void) smb_config_getstr(SMB_CI_WINS_EXCL, excludestr,
	    sizeof (excludestr));
	nexclude = smb_wins_iplist(excludestr, exclude, SMB_PI_MAX_NETWORKS);

	nc = smb_niclist.nl_nics;
	iflist = list_head(&hlist.h_list);

	do {
		for (i = 0; i < iflist->if_num; i++) {
			ifname = iflist->if_names[i];
			if (smb_nic_getinfo(ifname, nc) < 0)
				continue;

			(void) strlcpy(nc->nic_host, iflist->if_host,
			    sizeof (nc->nic_host));
			(void) strlcpy(nc->nic_cmnt, iflist->if_cmnt,
			    sizeof (nc->nic_cmnt));

			smb_tonetbiosname(nc->nic_host, nc->nic_nbname, 0x00);

			if (strchr(ifname, ':'))
				nc->nic_smbflags |= SMB_NICF_ALIAS;

			if (smb_wins_is_excluded(nc->nic_ip,
			    (ipaddr_t *)exclude, nexclude))
				nc->nic_smbflags |= SMB_NICF_NBEXCL;

			smb_niclist.nl_cnt++;
			nc++;
		}
	} while	((iflist = list_next(&hlist.h_list, iflist)) != NULL);

	smb_nic_hlist_destroy(&hlist);

	return (0);
}

static void
smb_nic_list_destroy(void)
{
	free(smb_niclist.nl_nics);
	smb_niclist.nl_nics = NULL;
	smb_niclist.nl_cnt = 0;
}

/*
 * smb_nic_getinfo
 *
 * Get IP info and more for the given interface
 */
static int
smb_nic_getinfo(char *interface, smb_nic_t *nc)
{
	struct lifreq lifrr;
	struct sockaddr_in *sa;
	int s;

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0) {
		return (-1);
	}

	(void) strlcpy(lifrr.lifr_name, interface, sizeof (lifrr.lifr_name));
	if (ioctl(s, SIOCGLIFADDR, &lifrr) < 0) {
		(void) close(s);
		return (-1);
	}
	sa = (struct sockaddr_in *)&lifrr.lifr_addr;
	nc->nic_ip = (uint32_t)sa->sin_addr.s_addr;

	if (nc->nic_ip == 0) {
		(void) close(s);
		return (-1);
	}

	if (ioctl(s, SIOCGLIFBRDADDR, &lifrr) < 0) {
		(void) close(s);
		return (-1);
	}
	sa = (struct sockaddr_in *)&lifrr.lifr_broadaddr;
	nc->nic_bcast = (uint32_t)sa->sin_addr.s_addr;

	if (ioctl(s, SIOCGLIFNETMASK, &lifrr) < 0) {
		(void) close(s);
		return (-1);
	}
	sa = (struct sockaddr_in *)&lifrr.lifr_addr;
	nc->nic_mask = (uint32_t)sa->sin_addr.s_addr;

	if (ioctl(s, SIOCGLIFFLAGS, &lifrr) < 0) {
		(void) close(s);
		return (-1);
	}
	nc->nic_sysflags = lifrr.lifr_flags;

	(void) strlcpy(nc->nic_ifname, interface, sizeof (nc->nic_ifname));

	(void) close(s);
	return (0);
}

/*
 * smb_nic_hlist_create
 *
 * Creates a list of hosts and their associated interfaces.
 * If host database exists the information is retrieved from
 * the database, otherwise it's retrieved from OS.
 *
 * The allocated memories for the returned list should be freed
 * by calling smb_nic_hlist_destroy()
 */
static int
smb_nic_hlist_create(smb_hosts_t *hlist)
{
	int rc;

	list_create(&hlist->h_list, sizeof (smb_hostifs_t),
	    offsetof(smb_hostifs_t, if_lnd));
	hlist->h_num = 0;
	hlist->h_ifnum = 0;

	if (smb_nic_dbexists() && smb_nic_dbvalidate()) {
		rc = smb_nic_hlist_dbget(hlist);
		errno = EBADF;
	} else {
		rc = smb_nic_hlist_sysget(hlist);
	}

	if (rc != 0)
		smb_nic_hlist_destroy(hlist);

	return (rc);
}

static void
smb_nic_hlist_destroy(smb_hosts_t *hlist)
{
	smb_hostifs_t *iflist;

	if (hlist == NULL)
		return;

	while ((iflist = list_head(&hlist->h_list)) != NULL) {
		list_remove(&hlist->h_list, iflist);
		smb_nic_iflist_destroy(iflist);
	}

	list_destroy(&hlist->h_list);
}

/*
 * smb_nic_hlist_sysget
 *
 * Get the list of currently plumbed and up interface names. The loopback (lo0)
 * port is ignored
 */
static int
smb_nic_hlist_sysget(smb_hosts_t *hlist)
{
	smb_hostifs_t *iflist;
	struct ifconf ifc;
	struct ifreq ifr;
	struct ifreq *ifrp;
	char *ifname;
	int ifnum;
	int i;
	int s;

	iflist = malloc(sizeof (smb_hostifs_t));
	if (iflist == NULL)
		return (-1);

	bzero(iflist, sizeof (smb_hostifs_t));

	if (smb_gethostname(iflist->if_host, sizeof (iflist->if_host), 0) < 0) {
		free(iflist);
		return (-1);
	}

	(void) smb_config_getstr(SMB_CI_SYS_CMNT, iflist->if_cmnt,
	    sizeof (iflist->if_cmnt));

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		free(iflist);
		return (-1);
	}

	if (ioctl(s, SIOCGIFNUM, (char *)&ifnum) < 0) {
		(void) close(s);
		free(iflist);
		return (-1);
	}

	ifc.ifc_len = ifnum * sizeof (struct ifreq);
	ifc.ifc_buf = malloc(ifc.ifc_len);
	if (ifc.ifc_buf == NULL) {
		(void) close(s);
		free(iflist);
		return (-1);
	}
	bzero(ifc.ifc_buf, ifc.ifc_len);

	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
		(void) close(s);
		free(iflist);
		free(ifc.ifc_buf);
		return (-1);
	}

	ifrp = ifc.ifc_req;
	ifnum = ifc.ifc_len / sizeof (struct ifreq);

	for (i = 0; i < ifnum; i++, ifrp++) {
		/*
		 * Get the flags so that we can skip the loopback interface
		 */
		(void) memset(&ifr, 0, sizeof (ifr));
		(void) strlcpy(ifr.ifr_name, ifrp->ifr_name,
		    sizeof (ifr.ifr_name));

		if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
			(void) close(s);
			free(ifc.ifc_buf);
			smb_nic_iflist_destroy(iflist);
			return (-1);
		}

		if (ifr.ifr_flags & IFF_LOOPBACK)
			continue;

		if ((ifr.ifr_flags & IFF_UP) == 0)
			continue;

		ifname = strdup(ifrp->ifr_name);
		if (ifname == NULL) {
			(void) close(s);
			free(ifc.ifc_buf);
			smb_nic_iflist_destroy(iflist);
			return (-1);
		}
		iflist->if_names[iflist->if_num++] = ifname;
	}

	(void) close(s);
	free(ifc.ifc_buf);

	hlist->h_num = 1;
	hlist->h_ifnum = iflist->if_num;
	list_insert_tail(&hlist->h_list, iflist);

	return (0);
}

static int
smb_nic_hlist_dbget(smb_hosts_t *hlist)
{
	smb_hostifs_t *iflist;
	sqlite *db;
	sqlite_vm *vm;
	boolean_t error = B_FALSE;
	const char **values;
	char *sql;
	char *errmsg = NULL;
	int ncol, rc;

	sql = sqlite_mprintf("SELECT * FROM hosts");
	if (sql == NULL)
		return (-1);

	db = smb_nic_dbopen(SMB_NIC_DB_ORD);
	if (db == NULL) {
		sqlite_freemem(sql);
		return (-1);
	}

	rc = sqlite_compile(db, sql, NULL, &vm, &errmsg);
	sqlite_freemem(sql);

	if (rc != SQLITE_OK) {
		smb_nic_dbclose(db);
		syslog(LOG_DEBUG, "smb_nic_hlist_dbget: failed to create"
		    " VM (%s)", NULL_MSGCHK(errmsg));
		return (-1);
	}

	do {
		rc = sqlite_step(vm, &ncol, &values, NULL);
		if (rc == SQLITE_ROW) {
			if (ncol != SMB_NIC_HTBL_NCOL) {
				error = B_TRUE;
				break;
			}

			if ((iflist = smb_nic_iflist_decode(values)) == NULL) {
				error = B_TRUE;
				break;
			}

			list_insert_tail(&hlist->h_list, iflist);
			hlist->h_num++;
			hlist->h_ifnum += iflist->if_num;
		}
	} while (rc == SQLITE_ROW);

	if (rc != SQLITE_DONE)
		error = B_TRUE;

	rc = sqlite_finalize(vm, &errmsg);
	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "smb_nic_hlist_dbget: failed to destroy"
		    "VM (%s)", NULL_MSGCHK(errmsg));
		error = B_TRUE;
	}

	smb_nic_dbclose(db);

	return ((error) ? -1 : 0);
}

static smb_hostifs_t *
smb_nic_iflist_decode(const char **values)
{
	smb_hostifs_t *iflist;
	char *host;
	char *cmnt;
	char *ifnames;
	char *lasts;
	char *ifname;
	int if_num = 0;

	host = (char *)values[SMB_NIC_HTBL_HOST];
	cmnt = (char *)values[SMB_NIC_HTBL_CMNT];
	ifnames = (char *)values[SMB_NIC_HTBL_IFS];

	if ((host == NULL) || (ifnames == NULL))
		return (NULL);

	iflist = malloc(sizeof (smb_hostifs_t));
	if (iflist == NULL)
		return (NULL);

	bzero(iflist, sizeof (smb_hostifs_t));

	(void) strlcpy(iflist->if_host, host, sizeof (iflist->if_host));
	(void) strlcpy(iflist->if_cmnt, (cmnt) ? cmnt : "",
	    sizeof (iflist->if_cmnt));

	if ((ifname = strtok_r(ifnames, ",", &lasts)) == NULL)
		return (NULL);

	iflist->if_names[if_num++] = strdup(ifname);

	while ((ifname = strtok_r(NULL, ",", &lasts)) != NULL)
		iflist->if_names[if_num++] = strdup(ifname);

	iflist->if_num = if_num;

	for (if_num = 0; if_num < iflist->if_num; if_num++) {
		if (iflist->if_names[if_num] == NULL) {
			smb_nic_iflist_destroy(iflist);
			return (NULL);
		}
	}

	return (iflist);
}

/*
 * smb_nic_iflist_destroy
 *
 * Frees allocated memory for the given IF names lists.
 */
static void
smb_nic_iflist_destroy(smb_hostifs_t *iflist)
{
	int i;

	if (iflist == NULL)
		return;

	for (i = 0; i < iflist->if_num; i++)
		free(iflist->if_names[i]);

	free(iflist);
}

/*
 * Functions to manage host/interface database
 *
 * Each entry in the hosts table associates a hostname with a
 * list of interface names. The host/interface association could
 * be added by calling smb_nic_addhost() and could be removed by
 * calling smb_nic_delhost(). If the database exists and it contains
 * valid information then the inteface list wouldn't be obtained
 * from system using ioctl.
 */

/*
 * smb_nic_dbcreate
 *
 * Creates the host database based on the defined SQL statement.
 * It also initializes db_info table.
 */
static int
smb_nic_dbcreate(void)
{
	sqlite *db = NULL;
	char *errmsg = NULL;
	int rc;

	(void) unlink(SMB_NIC_DB_NAME);

	db = sqlite_open(SMB_NIC_DB_NAME, 0600, &errmsg);
	if (db == NULL) {
		syslog(LOG_DEBUG, "failed to create host database (%s)",
		    NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
		return (SQLITE_CANTOPEN);
	}

	sqlite_busy_timeout(db, SMB_NIC_DB_TIMEOUT);
	rc = sqlite_exec(db, "BEGIN TRANSACTION", NULL, NULL, &errmsg);
	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "failed to begin database transaction (%s)",
		    NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
		sqlite_close(db);
		return (rc);
	}

	if (sqlite_exec(db, SMB_NIC_DB_SQL, NULL, NULL, &errmsg) == SQLITE_OK) {
		rc = sqlite_exec(db, "COMMIT TRANSACTION", NULL, NULL,
		    &errmsg);
		if (rc == SQLITE_OK)
			rc = smb_nic_dbsetinfo(db);
		if (rc != SQLITE_OK)
			rc = sqlite_exec(db, "ROLLBACK TRANSACTION", NULL, NULL,
			    &errmsg);
	} else {
		syslog(LOG_ERR, "failed to initialize host database (%s)",
		    errmsg);
		sqlite_freemem(errmsg);
		rc = sqlite_exec(db, "ROLLBACK TRANSACTION", NULL, NULL,
		    &errmsg);
	}

	if (rc != SQLITE_OK) {
		/* this is bad - database may be left in a locked state */
		syslog(LOG_DEBUG, "failed to close a transaction (%s)",
		    NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
	}

	(void) sqlite_close(db);
	return (rc);
}

/*
 * smb_nic_dbopen
 *
 * Opens host database with the given mode.
 */
static sqlite *
smb_nic_dbopen(int mode)
{
	sqlite *db;
	char *errmsg = NULL;

	db = sqlite_open(SMB_NIC_DB_NAME, mode, &errmsg);
	if (db == NULL) {
		syslog(LOG_ERR, "failed to open group database (%s)",
		    NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
	}

	return (db);
}

/*
 * smb_nic_dbclose
 *
 * Closes the given database handle
 */
static void
smb_nic_dbclose(sqlite *db)
{
	if (db) {
		sqlite_close(db);
	}
}

static boolean_t
smb_nic_dbexists(void)
{
	return (access(SMB_NIC_DB_NAME, F_OK) == 0);
}

static boolean_t
smb_nic_dbvalidate(void)
{
	sqlite *db;
	char *errmsg = NULL;
	char *sql;
	char **result;
	int nrow, ncol;
	boolean_t check = B_TRUE;
	int rc;

	sql = sqlite_mprintf("SELECT * FROM db_info");
	if (sql == NULL)
		return (B_FALSE);

	db = smb_nic_dbopen(SMB_NIC_DB_ORW);
	if (db == NULL) {
		sqlite_freemem(sql);
		return (B_FALSE);
	}

	rc = sqlite_get_table(db, sql, &result, &nrow, &ncol, &errmsg);
	sqlite_freemem(sql);

	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "smb_nic_dbvalidate: failed to get db_info"
		    " (%s)", NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
		smb_nic_dbclose(db);
		return (B_FALSE);
	}

	if (nrow != 1 || ncol != 3) {
		syslog(LOG_DEBUG, "smb_nic_dbvalidate: bad db_info table");
		sqlite_free_table(result);
		smb_nic_dbclose(db);
		return (B_FALSE);
	}

	if ((atoi(result[3]) != SMB_NIC_DB_VERMAJOR) ||
	    (atoi(result[4]) != SMB_NIC_DB_VERMINOR) ||
	    (atoi(result[5]) != SMB_NIC_DB_MAGIC)) {
		syslog(LOG_DEBUG, "smb_nic_dbvalidate: bad db_info content");
		sqlite_free_table(result);
		smb_nic_dbclose(db);
		return (B_FALSE);
	}
	sqlite_free_table(result);

	sql = sqlite_mprintf("SELECT hostname FROM hosts");
	if (sql == NULL) {
		smb_nic_dbclose(db);
		return (B_FALSE);
	}

	rc = sqlite_get_table(db, sql, &result, &nrow, &ncol, &errmsg);
	sqlite_freemem(sql);

	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "smb_nic_dbvalidate: failed to count (%s)",
		    NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
		smb_nic_dbclose(db);
		return (B_FALSE);
	}

	sqlite_free_table(result);

	if (nrow == 0)
		/* No hosts in the database */
		check = B_FALSE;

	smb_nic_dbclose(db);
	return (check);
}

static int
smb_nic_dbaddhost(const char *host, const char *cmnt, char *if_list)
{
	sqlite *db;
	char *sql;
	char *errmsg;
	int rc;

	sql = sqlite_mprintf("REPLACE INTO hosts (hostname, comment, ifnames)"
	    "VALUES ('%s', '%q', '%s')", host, (cmnt) ? cmnt : "", if_list);
	if (sql == NULL)
		return (SQLITE_NOMEM);

	db = smb_nic_dbopen(SMB_NIC_DB_ORW);
	if (db == NULL) {
		sqlite_freemem(sql);
		return (SQLITE_CANTOPEN);
	}

	rc = sqlite_exec(db, sql, NULL, NULL, &errmsg);
	sqlite_freemem(sql);
	smb_nic_dbclose(db);

	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "smb_nic_dbaddhost: failed to insert %s (%s)",
		    host, NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
	}

	return (rc);
}

static int
smb_nic_dbdelhost(const char *host)
{
	sqlite *db;
	char *sql;
	char *errmsg;
	int rc;

	sql = sqlite_mprintf("DELETE FROM hosts WHERE hostname = '%s'", host);
	if (sql == NULL)
		return (SQLITE_NOMEM);

	db = smb_nic_dbopen(SMB_NIC_DB_ORW);
	if (db == NULL) {
		sqlite_freemem(sql);
		return (SQLITE_CANTOPEN);
	}

	rc = sqlite_exec(db, sql, NULL, NULL, &errmsg);
	sqlite_freemem(sql);
	smb_nic_dbclose(db);

	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "smb_nic_dbdelhost: failed to delete %s (%s)",
		    host, NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
	}

	return (rc);
}

/*
 * smb_nic_dbsetinfo
 *
 * Initializes the db_info table upon database creation.
 */
static int
smb_nic_dbsetinfo(sqlite *db)
{
	char *errmsg = NULL;
	char *sql;
	int rc;

	sql = sqlite_mprintf("INSERT INTO db_info (ver_major, ver_minor,"
	    " magic) VALUES (%d, %d, %d)", SMB_NIC_DB_VERMAJOR,
	    SMB_NIC_DB_VERMINOR, SMB_NIC_DB_MAGIC);

	if (sql == NULL)
		return (SQLITE_NOMEM);

	rc = sqlite_exec(db, sql, NULL, NULL, &errmsg);
	sqlite_freemem(sql);
	if (rc != SQLITE_OK) {
		syslog(LOG_DEBUG, "smb_nic_dbsetinfo: failed to insert database"
		    " information (%s)", NULL_MSGCHK(errmsg));
		sqlite_freemem(errmsg);
	}

	return (rc);
}
