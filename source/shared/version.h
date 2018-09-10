#ifndef VERSION_H
#define VERSION_H
//---------------------------------------------------------------------------------------------------------------------------------
// File: version.h
// Contents: Version number constants
//
//---------------------------------------------------------------------------------------------------------------------------------

// helper macros to stringify the a macro value
#define STRINGIFY(a) TOSTRING(a)
#define TOSTRING(a) #a

// Increase Major number with backward incompatible breaking changes.
// Increase Minor with backward compatible new functionalities and API changes.
// Increase Patch for backward compatible fixes.
#define SQLVERSION_MAJOR 5
#define SQLVERSION_MINOR 3
#define SQLVERSION_PATCH 0
#define SQLVERSION_BUILD 0

// For previews, set this constant to 1. Otherwise, set it to 0
#define PREVIEW 0
#define SEMVER_PRERELEASE

// Semantic versioning build metadata, build meta data is not counted in precedence order.
#define SEMVER_BUILDMETA

#if SQLVERSION_BUILD > 0
#undef SEMVER_BUILDMETA
#define SEMVER_BUILDMETA "+" STRINGIFY( SQLVERSION_BUILD )
#endif

// Main version, dot separated 3 digits, Major.Minor.Patch
#define VER_APIVERSION_STR      STRINGIFY( SQLVERSION_MAJOR ) "." STRINGIFY( SQLVERSION_MINOR ) "." STRINGIFY( SQLVERSION_PATCH )

// Semantic versioning: 
// For stable releases leave SEMVER_PRERELEASE empty
// Otherwise, for pre-releases, add '-' and change it to:
// "RC" for release candidates
// "preview" for ETP 
#if PREVIEW > 0
#undef SEMVER_PRERELEASE
#define SEMVER_PRERELEASE "preview"
#define VER_FILEVERSION_STR     VER_APIVERSION_STR "-" SEMVER_PRERELEASE SEMVER_BUILDMETA
#else
#define VER_FILEVERSION_STR     VER_APIVERSION_STR SEMVER_PRERELEASE SEMVER_BUILDMETA
#endif

#define _FILEVERSION            SQLVERSION_MAJOR,SQLVERSION_MINOR,SQLVERSION_PATCH,SQLVERSION_BUILD

// PECL package version macros ('-' or '+' is not allowed)
#define PHP_HDB_VERSION      VER_APIVERSION_STR SEMVER_PRERELEASE
#define PHP_PDO_HDB_VERSION  PHP_HDB_VERSION

#endif // VERSION_H
