//---------------------------------------------------------------------------------------------------------------------------------
// File: core_conn.cpp
//
// Contents: Core routines that use connection handles shared between hdb and pdo_hdb
//
//---------------------------------------------------------------------------------------------------------------------------------

#include "core_hdb.h"

#include <php.h>

#include <sstream>
#include <vector>
#include <sys/utsname.h>

// *** internal variables and constants ***

namespace {

// *** internal constants ***
// an arbitrary figure that should be large enough for most connection strings.
const int DEFAULT_CONN_STR_LEN = 2048;

// length of buffer used to retrieve information for client and server info buffers
const int INFO_BUFFER_LEN = 256;

// length for name of keystore used in CEKeyStoreData
const int MAX_CE_NAME_LEN = 260;

// processor architectures
const char* PROCESSOR_ARCH[] = { "x86", "x64", "ia64" };

const char* HDB_DRIVER_NAME = "{HDBODBC}";

// ODBC driver names.
// the order of this list should match the order of DRIVER_VERSION enum
std::vector<std::string> CONNECTION_STRING_DRIVER_NAME{ "Driver={ODBC Driver 17 for SQL Server};", "Driver={ODBC Driver 13 for SQL Server};", "Driver={ODBC Driver 11 for SQL Server};" };

// default options if only the server is specified
const char CONNECTION_STRING_DEFAULT_OPTIONS[] = "Mars_Connection={Yes};";

// connection option appended when no user name or password is given
const char CONNECTION_OPTION_NO_CREDENTIALS[] = "Trusted_Connection={Yes};";

// connection option appended for MARS when MARS isn't explicitly mentioned
const char CONNECTION_OPTION_MARS_ON[] = "MARS_Connection={Yes};";

// *** internal function prototypes ***

void build_connection_string_and_set_conn_attr( _Inout_ hdb_conn* conn, _Inout_z_ const char* server, _Inout_opt_z_ const char* uid, _Inout_opt_z_ const char* pwd,
                                                _Inout_opt_ HashTable* options_ht, _In_ const connection_option valid_conn_opts[],
                                                void* driver,_Inout_ std::string& connection_string TSRMLS_DC );
connection_option const* get_connection_option( hdb_conn* conn, _In_ const char* key, _In_ SQLULEN key_len TSRMLS_DC );
void common_conn_str_append_func( _In_z_ const char* odbc_name, _In_reads_(val_len) const char* val, _Inout_ size_t val_len, _Inout_ std::string& conn_str TSRMLS_DC );
}

// core_hdb_connect
// opens a connection and returns a hdb_conn structure.
// Parameters:
// henv_cp           - connection pooled env context
// henv_ncp          - non connection pooled env context
// server            - name of the server we're connecting to
// uid               - username
// pwd               - password
// options_ht        - zend_hash list of options
// err               - error callback to put into the connection's context
// valid_conn_opts[] - array of valid driver supported connection options.
// driver            - reference to caller
// Return
// A hdb_conn structure. An exception is thrown if an error occurs

hdb_conn* core_hdb_connect( _In_ hdb_context& henv_cp, _In_ hdb_context& henv_ncp, _In_ driver_conn_factory conn_factory,
                                  _Inout_z_ const char* server, _Inout_opt_z_ const char* uid, _Inout_opt_z_ const char* pwd,
                                  _Inout_opt_ HashTable* options_ht, _In_ error_callback err, _In_ const connection_option valid_conn_opts[],
                                  _In_ void* driver, _In_z_ const char* driver_func TSRMLS_DC )

{
    SQLRETURN r;
    std::string conn_str;
    conn_str.reserve( DEFAULT_CONN_STR_LEN );
    hdb_malloc_auto_ptr<hdb_conn> conn;
    bool is_pooled = false;

    hdb_context* henv = &henv_ncp;  // by default do not use the connection pooling henv

    try {
        SQLHANDLE temp_conn_h;
        core::SQLAllocHandle( SQL_HANDLE_DBC, *henv, &temp_conn_h TSRMLS_CC );
        conn = conn_factory( temp_conn_h, err, driver TSRMLS_CC );
        conn->set_func( driver_func );

        build_connection_string_and_set_conn_attr( conn, server, uid, pwd, options_ht, valid_conn_opts, driver, conn_str TSRMLS_CC );

        r = core_odbc_connect( conn, conn_str, is_pooled );
        CHECK_SQL_ERROR( r, conn ) {
            throw core::CoreException();
        }

        CHECK_SQL_WARNING_AS_ERROR( r, conn ) {
            throw core::CoreException();
        }
    }
    catch( std::bad_alloc& ) {
        conn_str.clear();
        conn->invalidate();
        DIE( "C++ memory allocation failure building the connection string." );
    }
    catch( std::out_of_range const& ex ) {
        conn_str.clear();
        LOG( SEV_ERROR, "C++ exception returned: %1!s!", ex.what() );
        conn->invalidate();
        throw;
    }
    catch( std::length_error const& ex ) {
        conn_str.clear();
        LOG( SEV_ERROR, "C++ exception returned: %1!s!", ex.what() );
        conn->invalidate();
        throw;
    }
    catch( core::CoreException&  ) {
        conn_str.clear();
        conn->invalidate();
        throw;
    }

    conn_str.clear();
    hdb_conn* return_conn = conn;
    conn.transferred();

    return return_conn;
}

// core_compare_error_state
// This method compares the error state to the one specified
// Parameters:
// conn         - the connection structure on which we establish the connection
// rc           - ODBC return code
// Return       - a boolean flag that indicates if the error states are the same

bool core_compare_error_state( _In_ hdb_conn* conn,  _In_ SQLRETURN rc, _In_ const char* error_state )
{
    if( SQL_SUCCEEDED( rc ) )
        return false;

    SQLCHAR state[ SQL_SQLSTATE_BUFSIZE ] = { 0 };
    SQLSMALLINT len;
    SQLRETURN sr = SQLGetDiagField( SQL_HANDLE_DBC, conn->handle(), 1, SQL_DIAG_SQLSTATE, state, SQL_SQLSTATE_BUFSIZE, &len );

    return ( SQL_SUCCEEDED(sr) && ! strcmp(error_state, reinterpret_cast<char*>( state ) ) );
}

// core_odbc_connect
// calls odbc connect API to establish the connection to server
// Parameters:
// conn                 - The connection structure on which we establish the connection
// conn_str             - Connection string
// is_pooled            - indicate whether it is a pooled connection
// Return               - SQLRETURN status returned by SQLDriverConnect

SQLRETURN core_odbc_connect( _Inout_ hdb_conn* conn, _Inout_ std::string& conn_str, _In_ bool is_pooled )
{
    SQLRETURN r = SQL_SUCCESS;
    //hdb_malloc_auto_ptr<SQLWCHAR> wconn_string;
    //unsigned int wconn_len = static_cast<unsigned int>( conn_str.length() + 1 ) * sizeof( SQLWCHAR );

    // We only support UTF-8 encoding for connection string.
    // Convert our UTF-8 connection string to UTF-16 before connecting with SQLDriverConnnectW
    //wconn_string = utf16_string_from_mbcs_string( HDB_ENCODING_UTF8, conn_str.c_str(), static_cast<unsigned int>( conn_str.length() ), &wconn_len );

    //CHECK_CUSTOM_ERROR( wconn_string == 0, conn, HDB_ERROR_CONNECT_STRING_ENCODING_TRANSLATE, get_last_error_message())
    //{
    //    throw core::CoreException();
    //}

    SQLSMALLINT output_conn_size;
    r = SQLSetEnvAttr( conn->handle(), SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    //r = SQLDriverConnectW( conn->handle(), NULL, wconn_string, static_cast<SQLSMALLINT>( wconn_len ), NULL, 0, &output_conn_size, SQL_DRIVER_NOPROMPT );
    r = SQLDriverConnect( conn->handle(), NULL, (SQLCHAR*)conn_str.c_str(), SQL_NTS, NULL, 0, &output_conn_size, SQL_DRIVER_NOPROMPT );

    // clear the connection string from memory
    //memset( wconn_string, 0, wconn_len * sizeof( SQLWCHAR )); // wconn_len is the number of characters, not bytes
    conn_str.clear();

    return r;
}


// core_hdb_begin_transaction
// Begins a transaction on a specified connection. The current transaction
// includes all statements on the specified connection that were executed after
// the call to core_hdb_begin_transaction and before any calls to
// core_hdb_rollback or core_hdb_commit.
// The default transaction mode is auto-commit. This means that all queries
// are automatically committed upon success unless they have been designated
// as part of an explicit transaction by using core_hdb_begin_transaction.
// Parameters:
// hdb_conn*: The connection with which the transaction is associated.

void core_hdb_begin_transaction( _Inout_ hdb_conn* conn TSRMLS_DC )
{
    try {

        DEBUG_HDB_ASSERT( conn != NULL, "core_hdb_begin_transaction: connection object was null." );

        core::SQLSetConnectAttr( conn, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>( SQL_AUTOCOMMIT_OFF ),
                                 SQL_IS_UINTEGER TSRMLS_CC );
    }
    catch ( core::CoreException& ) {
        throw;
    }
}

// core_hdb_commit
// Commits the current transaction on the specified connection and returns the
// connection to the auto-commit mode. The current transaction includes all
// statements on the specified connection that were executed after the call to
// core_hdb_begin_transaction and before any calls to core_hdb_rollback or
// core_hdb_commit.
// Parameters:
// hdb_conn*: The connection on which the transaction is active.

void core_hdb_commit( _Inout_ hdb_conn* conn TSRMLS_DC )
{
    try {

        DEBUG_HDB_ASSERT( conn != NULL, "core_hdb_commit: connection object was null." );

        core::SQLEndTran( SQL_HANDLE_DBC, conn, SQL_COMMIT TSRMLS_CC );

        core::SQLSetConnectAttr( conn, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>( SQL_AUTOCOMMIT_ON ),
                                SQL_IS_UINTEGER TSRMLS_CC );
    }
    catch ( core::CoreException& ) {
        throw;
    }
}

// core_hdb_rollback
// Rolls back the current transaction on the specified connection and returns
// the connection to the auto-commit mode. The current transaction includes all
// statements on the specified connection that were executed after the call to
// core_hdb_begin_transaction and before any calls to core_hdb_rollback or
// core_hdb_commit.
// Parameters:
// hdb_conn*: The connection on which the transaction is active.

void core_hdb_rollback( _Inout_ hdb_conn* conn TSRMLS_DC )
{
    try {

        DEBUG_HDB_ASSERT( conn != NULL, "core_hdb_rollback: connection object was null." );

        core::SQLEndTran( SQL_HANDLE_DBC, conn, SQL_ROLLBACK TSRMLS_CC );

        core::SQLSetConnectAttr( conn, SQL_ATTR_AUTOCOMMIT, reinterpret_cast<SQLPOINTER>( SQL_AUTOCOMMIT_ON ),
                                 SQL_IS_UINTEGER TSRMLS_CC );

    }
    catch ( core::CoreException& ) {
        throw;
    }
}

// core_hdb_close
// Called when a connection resource is destroyed by the Zend engine.
// Parameters:
// conn - The current active connection.
void core_hdb_close( _Inout_opt_ hdb_conn* conn TSRMLS_DC )
{
    // if the connection wasn't successful, just return.
    if( conn == NULL )
        return;

    try {

        // rollback any transaction in progress (we don't care about the return result)
        core::SQLEndTran( SQL_HANDLE_DBC, conn, SQL_ROLLBACK TSRMLS_CC );
    }
    catch( core::CoreException& ) {
        LOG( SEV_ERROR, "Transaction rollback failed when closing the connection." );
    }

    // disconnect from the server
    SQLRETURN r = SQLDisconnect( conn->handle() );
    if( !SQL_SUCCEEDED( r )) {
        LOG( SEV_ERROR, "Disconnect failed when closing the connection." );
    }

    // free the connection handle
    conn->invalidate();

    hdb_free( conn );
}

// core_hdb_prepare
// Create a statement object and prepare the SQL query passed in for execution at a later time.
// Parameters:
// stmt - statement to be prepared
// sql - T-SQL command to prepare
// sql_len - length of the T-SQL string

void core_hdb_prepare( _Inout_ hdb_stmt* stmt, _In_reads_bytes_(sql_len) const char* sql, _In_ SQLLEN sql_len TSRMLS_DC )
{
    try {

        // convert the string from its encoding to UTf-16
        // if the string is empty, we initialize the fields and skip since an empty string is a
        // failure case for utf16_string_from_mbcs_string
        hdb_malloc_auto_ptr<SQLWCHAR> wsql_string;
        unsigned int wsql_len = 0;
        if( sql_len == 0 || ( sql[0] == '\0' && sql_len == 1 )) {
            wsql_string = reinterpret_cast<SQLWCHAR*>( hdb_malloc( sizeof( SQLWCHAR )));
            wsql_string[0] = L'\0';
            wsql_len = 0;
        }
        else {
             if( sql_len > INT_MAX ) {
                LOG( SEV_ERROR, "Convert input parameter to utf16: buffer length exceeded.");
                throw core::CoreException();
             }

             HDB_ENCODING encoding = (( stmt->encoding() == HDB_ENCODING_DEFAULT ) ? stmt->conn->encoding() : stmt->encoding() );
             wsql_string = utf16_string_from_mbcs_string( encoding, reinterpret_cast<const char*>( sql ), static_cast<int>( sql_len ), &wsql_len );
             CHECK_CUSTOM_ERROR( wsql_string == 0, stmt, HDB_ERROR_QUERY_STRING_ENCODING_TRANSLATE, get_last_error_message() ) {
                 throw core::CoreException();
             }
        }

        // prepare our wide char query string
        core::SQLPrepareW( stmt, reinterpret_cast<SQLWCHAR*>( wsql_string.get() ), wsql_len TSRMLS_CC );

        stmt->param_descriptions.clear();

        // if AE is enabled, get meta data for all parameters before binding them
        if( stmt->conn->ce_option.enabled ) {
            SQLSMALLINT num_params;
            core::SQLNumParams( stmt, &num_params);
            for( int i = 0; i < num_params; i++ ) {
                param_meta_data param;

                core::SQLDescribeParam( stmt, i + 1, &( param.sql_type ), &( param.column_size ), &( param.decimal_digits ), &( param.nullable ) );

                stmt->param_descriptions.push_back( param );
            }
        }
    }
    catch( core::CoreException& ) {

        throw;
    }
}

// core_is_conn_opt_value_escaped
// determine if connection string value is properly escaped.
// Properly escaped means that any '}' should be escaped by a prior '}'.  It is assumed that
// the value will be surrounded by { and } by the caller after it has been validated

bool core_is_conn_opt_value_escaped( _Inout_ const char* value, _Inout_ size_t value_len )
{
    // if the value is already quoted, then only analyse the part inside the quotes and return it as
    // unquoted since we quote it when adding it to the connection string.
    if( value_len > 0 && value[0] == '{' && value[ value_len - 1 ] == '}' ) {
        ++value;
        value_len -= 2;
    }
    // check to make sure that all right braces are escaped
    size_t i = 0;
    while( ( value[i] != '}' || ( value[i] == '}' && value[i+1] == '}' )) && i < value_len ) {
        // skip both braces
        if( value[i] == '}' )
            ++i;
        ++i;
    }
    if( i < value_len && value[i] == '}' ) {
        return false;
    }

    return true;
}

// core_is_authentication_option_valid
// if the option for the authentication is valid, returns true. This returns false otherwise.
bool core_is_authentication_option_valid( _In_z_ const char* value, _In_ size_t value_len)
{
    if (value_len <= 0)
        return false;

    if( ! stricmp( value, AzureADOptions::AZURE_AUTH_SQL_PASSWORD ) || ! stricmp( value, AzureADOptions::AZURE_AUTH_AD_PASSWORD ) ) {
        return true;
    }

    return false;
}


// *** internal connection functions and classes ***

namespace {

connection_option const* get_connection_option( hdb_conn* conn, _In_ SQLULEN key,
                                                     _In_ const connection_option conn_opts[] TSRMLS_DC )
{
    for( int opt_idx = 0; conn_opts[ opt_idx ].conn_option_key != HDB_CONN_OPTION_INVALID; ++opt_idx ) {

        if( key == conn_opts[ opt_idx ].conn_option_key ) {

            return &conn_opts[ opt_idx ];
         }
    }

    HDB_ASSERT( false, "Invalid connection option, should have been validated by the driver layer." );
    return NULL;    // avoid a compiler warning
}

// says what it does, and does what it says
// rather than have attributes and connection strings as ODBC does, we unify them into a hash table
// passed to the connection, and then break them out ourselves and either set attributes or put the
// option in the connection string.

void build_connection_string_and_set_conn_attr( _Inout_ hdb_conn* conn, _Inout_z_ const char* server, _Inout_opt_z_  const char* uid, 
                                                _Inout_opt_z_ const char* pwd,
                                                _Inout_opt_ HashTable* options, _In_ const connection_option valid_conn_opts[],
                                                void* driver, _Inout_ std::string& connection_string TSRMLS_DC )
{
    bool mars_mentioned = false;
    connection_option const* conn_opt;

    try {

        // Add the server name
        common_conn_str_append_func( ODBCConnOptions::Driver, HDB_DRIVER_NAME, strnlen_s( HDB_DRIVER_NAME ), connection_string TSRMLS_CC );
        common_conn_str_append_func( ODBCConnOptions::SERVER, server, strnlen_s( server ), connection_string TSRMLS_CC );

        bool escaped = core_is_conn_opt_value_escaped( uid, strnlen_s( uid ));
        CHECK_CUSTOM_ERROR( !escaped, conn, HDB_ERROR_UID_PWD_BRACES_NOT_ESCAPED ) {
            throw core::CoreException();
        }

        common_conn_str_append_func( ODBCConnOptions::UID, uid, strnlen_s( uid ), connection_string TSRMLS_CC );

        if( pwd != NULL ) {
            escaped = core_is_conn_opt_value_escaped( pwd, strnlen_s( pwd ));
            CHECK_CUSTOM_ERROR( !escaped, conn, HDB_ERROR_UID_PWD_BRACES_NOT_ESCAPED ) {
                throw core::CoreException();
            }

            common_conn_str_append_func( ODBCConnOptions::PWD, pwd, strnlen_s( pwd ), connection_string TSRMLS_CC );
        }

        // if no options were given, then we set MARS the defaults and return immediately.
        if( options == NULL || zend_hash_num_elements( options ) == 0 ) {
            connection_string += CONNECTION_STRING_DEFAULT_OPTIONS;
            return;
        }

        //zend_string *key = NULL;
        //zend_ulong index = -1;
        //zval* data = NULL;

        //ZEND_HASH_FOREACH_KEY_VAL( options, index, key, data ) {
        //    int type = HASH_KEY_NON_EXISTENT;
        //    type = key ? HASH_KEY_IS_STRING : HASH_KEY_IS_LONG;

        //    // The driver layer should ensure a valid key.
        //    DEBUG_HDB_ASSERT(( type == HASH_KEY_IS_LONG ), "build_connection_string_and_set_conn_attr: invalid connection option key type." );

        //    conn_opt = get_connection_option( conn, index, valid_conn_opts TSRMLS_CC );

        //    if( index == HDB_CONN_OPTION_MARS ) {
        //        mars_mentioned = true;
        //    }

        //    conn_opt->func( conn_opt, data, conn, connection_string TSRMLS_CC );
        //} ZEND_HASH_FOREACH_END();


    }
    catch( core::CoreException& ) {
        throw;
    }
}

void common_conn_str_append_func( _In_z_ const char* odbc_name, _In_reads_(val_len) const char* val, _Inout_ size_t val_len, _Inout_ std::string& conn_str TSRMLS_DC )
{
    // wrap a connection option in a quote.  It is presumed that any character that need to be escaped will
    // be escaped, such as a closing }.
    TSRMLS_C;

    //if( val_len > 0 && val[0] == '{' && val[ val_len - 1 ] == '}' ) {
    //    ++val;
    //    val_len -= 2;
    //}
    conn_str += odbc_name;
    conn_str += "=";
    conn_str.append( val, val_len );
    conn_str += ";";
}

}   // namespace

// simply add the parsed value to the connection string
void conn_str_append_func::func( _In_ connection_option const* option, _In_ zval* value, hdb_conn* /*conn*/, _Inout_ std::string& conn_str TSRMLS_DC )
{
    const char* val_str = Z_STRVAL_P( value );
    size_t val_len = Z_STRLEN_P( value );
    common_conn_str_append_func( option->odbc_name, val_str, val_len, conn_str TSRMLS_CC );
}

// do nothing for connection pooling since we handled it earlier when
// deciding which environment handle to use.
void conn_null_func::func( connection_option const* /*option*/, zval* /*value*/, hdb_conn* /*conn*/, std::string& /*conn_str*/ TSRMLS_DC )
{
    TSRMLS_C;
}

void driver_set_func::func( _In_ connection_option const* option, _In_ zval* value, _Inout_ hdb_conn* conn, _Inout_ std::string& conn_str TSRMLS_DC )
{
    const char* val_str = Z_STRVAL_P( value );
    size_t val_len = Z_STRLEN_P( value );
    std::string driver_option( "" );
    common_conn_str_append_func( option->odbc_name, val_str, val_len, driver_option TSRMLS_CC );

    conn->driver_version = ODBC_DRIVER_UNKNOWN;
    for ( short i = DRIVER_VERSION::FIRST; i <= DRIVER_VERSION::LAST && conn->driver_version == ODBC_DRIVER_UNKNOWN; ++i ) {
        std::string driver_name = CONNECTION_STRING_DRIVER_NAME[i];

        if (! driver_name.compare( driver_option ) ) {
            conn->driver_version = DRIVER_VERSION( i );
        }
    }

    CHECK_CUSTOM_ERROR( conn->driver_version == ODBC_DRIVER_UNKNOWN, conn, HDB_ERROR_CONNECT_INVALID_DRIVER, val_str) {
        throw core::CoreException();
    }

    conn_str += driver_option;
}

void column_encryption_set_func::func( _In_ connection_option const* option, _In_ zval* value, _Inout_ hdb_conn* conn, _Inout_ std::string& conn_str TSRMLS_DC )
{
    convert_to_string( value );
    const char* value_str = Z_STRVAL_P( value );

    // Column Encryption is disabled by default unless it is explicitly 'Enabled'
    conn->ce_option.enabled = false;
    if ( !stricmp(value_str, "enabled" )) {
        conn->ce_option.enabled = true;
    }

    conn_str += option->odbc_name;
    conn_str += "=";
    conn_str += value_str;
    conn_str += ";";
}

// helper function to evaluate whether a string value is true or false.
// Values = ("true" or "1") are treated as true values. Everything else is treated as false.
// Returns 1 for true and 0 for false.

size_t core_str_zval_is_true( _Inout_ zval* value_z )
{
    HDB_ASSERT( Z_TYPE_P( value_z ) == IS_STRING, "core_str_zval_is_true: This function only accepts zval of type string." );

    char* value_in = Z_STRVAL_P( value_z );
    size_t val_len = Z_STRLEN_P( value_z );

    // strip any whitespace at the end (whitespace is the same value in ASCII and UTF-8)
    size_t last_char = val_len - 1;
    while( isspace(( unsigned char )value_in[ last_char ] )) {
        value_in[ last_char ] = '\0';
        val_len = last_char;
        --last_char;
    }

    // save adjustments to the value made by stripping whitespace at the end
    Z_STRLEN_P( value_z ) = val_len;

    const char VALID_TRUE_VALUE_1[] = "true";
    const char VALID_TRUE_VALUE_2[] = "1";

    if(( val_len == ( sizeof( VALID_TRUE_VALUE_1 ) - 1 ) && !strnicmp( value_in, VALID_TRUE_VALUE_1, val_len )) ||
       ( val_len == ( sizeof( VALID_TRUE_VALUE_2 ) - 1 ) && !strnicmp( value_in, VALID_TRUE_VALUE_2, val_len ))
      ) {

         return 1; // true
    }

    return 0; // false
}
