
/*!
  @file           sqlsdbodbc.h
  @ingroup        ODBC
  @brief          extra attributes (SAP HANA ODBC driver specific)

\if EMIT_LICENCE

    ========== licence begin  SAP

    Copyright (c) 2001-2017 SAP AG

    All rights reserved.

    ========== licence end




\endif
*/
#ifndef ODBC_SBDOCBC_H
#define ODBC_SBDOCBC_H


/* enumerations for attribute values */

/* attributes for SQL_COMPONENT */
enum sql_sdb_component {
    SQL_COMPONENT_CAL = 0,
    SQL_COMPONENT_CPC,
    SQL_COMPONENT_CON,
    SQL_COMPONENT_DOM,
    SQL_COMPONENT_LOA,
    SQL_COMPONENT_ODB,
    SQL_COMPONENT_QUE,
    SQL_COMPONENT_SQL,
    SQL_COMPONENT_TPL,
    SQL_COMPONENT_UTI,
    SQL_COMPONENT_XCI
};

/* attributes for SQL_MODE */
enum sql_sdb_sqlmode {
    SQL_SQLMODE_EMPTY = 0,
    SQL_SQLMODE_INTERNAL,
    SQL_SQLMODE_DB2,
    SQL_SQLMODE_ANSI,
    SQL_SQLMODE_ORACLE,
    SQL_SQLMODE_SAPR3};

/* attributes for SQL_ATTR_AGGREGATE_TYPES (data types for expressions like SUM, AVG, etc.) */
enum sql_sdb_aggregatetypes {
    SQL_AGGREGATETYPES_FLOAT = 0,   /* always float numbers */
    SQL_AGGREGATETYPES_ORIGIN };    /* data type matching to the original column (MS-SQL like) */

/* attributes for SQL_ATTR_CONNECTTYPES (cmp. connect property CONNECTTYPE of SQLDBC) */
enum sql_sdb_connecttypes {
    SQL_CONNECTTYPES_OLTP = 0,   /* OLTP - The connection is an OLTP connection. (default) */
    SQL_CONNECTTYPES_LVC,        /* LVC  - The connection is used for liveCache procedure calls. */
    SQL_CONNECTTYPES_DBANA };    /* DBAnalyzer - connect to a particular task */

/* **************************************** */


#define SQL_CONNECT_OPT_DRVR_START 1000

/* driver defined connect options.
 */
#define SQL_ATTR_SQLMODE         SQL_CONNECT_OPT_DRVR_START+2

/* sets the component for the connection (see SQLDBC connect properties) */
#define SQL_ATTR_COMPONENT       SQL_CONNECT_OPT_DRVR_START+3

/* Read only connect attribute to check for NI connections */
#define SQL_IS_NI_CONNECTION SQL_CONNECT_OPT_DRVR_START+9

/* Set SAP Router String for SQLCancel over NI Connection */
#define SQL_CANCEL_ROUTER_STRING SQL_CONNECT_OPT_DRVR_START+10

/* Set session-timeout for connection */
#define SQL_ATTR_SESSION_TIMEOUT SQL_CONNECT_OPT_DRVR_START+11

/* overwrites the member sp1c_producer of the segement header. See vsp001 for 
   possible values */
#define SQL_ATTR_PRODUCER        SQL_CONNECT_OPT_DRVR_START+12

/* Codepage used for UCS2<->ASCII conversion. Codepage is selected from the DB.
   In Windows: default codepage is active codepage of the client.
   In non-Windows: default codepage is WINDOWS-1252 (contains Euro-symbol). */
/* a codepage is valid for all(!) connections of the affected driver. If
   SQLSetConnectAttr is called a second time with a different codepage,
   the driver could crash, if a conversion is simultaneously running. */
#define SQL_ATTR_CODEPAGE   SQL_CONNECT_OPT_DRVR_START+13

/* for this attribute SQL_TRUE is returned, if the DB supports Unicode,
   otherwise SQL_FALSE */
#define SQL_ATTR_UNICODEDB   SQL_CONNECT_OPT_DRVR_START+14

/* encryption for the connection. 0 means no encryption. */
#define SQL_ATTR_ENCRYPT   SQL_CONNECT_OPT_DRVR_START+15

/* Aggregate types for expressions (AVG, SUM, etc.). 1 means like MS-SQL
   i.e. SUM(INTEGER) is of type INTEGER (otherwise it would be FLOAT)
*/
#define SQL_ATTR_AGGREGATE_TYPES   SQL_CONNECT_OPT_DRVR_START+16

/* The character representing non-convertible UCS2 characters in ASCII strings.
   This property only works if property CODEPAGENAME is set.
   If this property is not set, a non-convertible UCS2 character causes an conversion error 
   when converting to ASCII.
   Setting this character to '\0' disables this feature (default setting is '?'). */
#define SQL_ATTR_DEFAULTMAPCHAR    SQL_CONNECT_OPT_DRVR_START+17

/* Connect types for a connection
   (cmp. connect property CONNECTTYPE of SQLDBC) */
#define SQL_ATTR_CONNECTTYPES   SQL_CONNECT_OPT_DRVR_START+18

#define SQL_ATTR_ENCRYPTION_OFF   0    /* no encryption */
#define SQL_ATTR_ENCRYPTION_SSL   1    /* SSL */


/* this value for SQL_ATTR_SESSION_TIMEOUT means that no 'TIMEOUT <to>'
 * is appended to connect string */
#define SQL_DEFAULT_SESSION_TIMEOUT  ((UDWORD) (1 << 30))

/* Check if servernode of previous SQLDriverConnect is a SAP router string */
#define SQL_DIAG_IS_NI_CONNECTION  (1002)

/* **************************************** */

/* new extensions of SQLDBC based ODBC */

/* for getting the SQLDBC-handle from an ODBC-connection */
#define SQL_SQLDBC_CONNECTION_HANDLE   (SQL_CONNECT_OPT_DRVR_START+10000)
#define SQL_SQLDBC_ENVIRONMENT_HANDLE  (SQL_CONNECT_OPT_DRVR_START+10001)

/* input parameters with SQL_C_CHAR are handled as UTF8 */
#define SQL_SQLDBC_CHAR_AS_UTF8        (SQL_CONNECT_OPT_DRVR_START+10002)

/* client info (sent via SQLDBC to the kernel) */
#define SQL_SQLDBC_SET_CLIENTINFO_KEY        (SQL_CONNECT_OPT_DRVR_START+10003)
#define SQL_SQLDBC_SET_CLIENTINFO_VALUE      (SQL_CONNECT_OPT_DRVR_START+10004)
#define SQL_SQLDBC_GET_CLIENTINFO_VALUE      (SQL_CONNECT_OPT_DRVR_START+10005)

/* boolean parameter to tell the ODBC driver whether SQL catalogs are
   visible or not. The default is TRUE what means that the catalog is visible. */
#define SQL_SQLDBC_SHOW_CATALOGS        (SQL_CONNECT_OPT_DRVR_START+10006)

/* boolean parameter to tell the ODBC driver to report BINTEXT fields
   as either NCLOB (default, TRUE) or LONG VARBINARY (FALSE) */
#define SQL_SQLDBC_BINTEXT_IS_NCLOB       (SQL_CONNECT_OPT_DRVR_START+10007)

/* Encoding of SQL_C_CHAR data */
#define SQL_SQLDBC_CHAR_AS_CESU8          (SQL_CONNECT_OPT_DRVR_START+10008)

/* switch for the spatial type returned for ST_POINT / GT_GEOMETRY columns
 *   2 = SQL_TYPE_DST_GEOMETRY (0x7474)                     [ODBC DEFAULT]
 *   1 = SQL_TYPE_ST_POINT (75) / SQL_TYPE_ST_GEOMETRY (74)
 *   0 = SQL_VARBINARY (-3)                                 [SQLDBC DEFAULT]
 */
#define SQL_SQLDBC_SPATIALTYPES         (SQL_CONNECT_OPT_DRVR_START+10009)

/* GIS types */
#define SQL_TYPE_ST_POINT     75
#define SQL_TYPE_ST_GEOMETRY  74
#define SQL_TYPE_DST_GEOMETRY 0x7474   /* Default type for ST_POINT and ST_GEOMETRY */

#define SQL_SQLDBC_PROXY_HOST           (SQL_CONNECT_OPT_DRVR_START+10010)
#define SQL_SQLDBC_PROXY_PORT           (SQL_CONNECT_OPT_DRVR_START+10011)
#define SQL_SQLDBC_PROXY_USERID         (SQL_CONNECT_OPT_DRVR_START+10012)
#define SQL_SQLDBC_PROXY_PASSWORD       (SQL_CONNECT_OPT_DRVR_START+10013)

#define SQL_SQLDBC_PROXY_SCP_ACCOUNT    (SQL_CONNECT_OPT_DRVR_START+10015)

/* Define a WebSocket URL and use the WebSocket protocol */
#define SQL_SQLDBC_WEBSOCKETURL         (SQL_CONNECT_OPT_DRVR_START+10017)


/*
 * Driver defined statement attributes.
 */

/*
 * Retreive the next print line from the SQLSCRIPT_PRINT library.
 *
 * SQLGetStmtAttr(W) will return SQL_NO_DATA when no lines remain.
 *
 * If the provided buffer is not large enough to store the line, it will be
 * truncated in the usual manner. It is not possible to repeat the call to
 * retrieve the untruncated line. To avoid truncation, provide a buffer with
 * sufficient space to store 32767 characters.
 */
#define SQL_ATTR_PRINTLINE              (SQL_CONNECT_OPT_DRVR_START+11000)

#endif /* of ODBC_SBDOCBC_H */
