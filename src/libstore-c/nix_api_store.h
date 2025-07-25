#ifndef NIX_API_STORE_H
#define NIX_API_STORE_H
/**
 * @defgroup libstore libstore
 * @brief C bindings for nix libstore
 *
 * libstore is used for talking to a Nix store
 * @{
 */
/** @file
 * @brief Main entry for the libstore C bindings
 */

#include "nix_api_util.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

/** @brief Reference to a Nix store */
typedef struct Store Store;
/** @brief Nix store path */
typedef struct StorePath StorePath;

/**
 * @brief Initializes the Nix store library
 *
 * This function should be called before creating a Store
 * This function can be called multiple times.
 *
 * @param[out] context Optional, stores error information
 * @return NIX_OK if the initialization was successful, an error code otherwise.
 */
nix_err nix_libstore_init(nix_c_context * context);

/**
 * @brief Like nix_libstore_init, but does not load the Nix configuration.
 *
 * This is useful when external configuration is not desired, such as when running unit tests.
 */
nix_err nix_libstore_init_no_load_config(nix_c_context * context);

/**
 * @brief Open a nix store.
 *
 * Store instances may share state and resources behind the scenes.
 *
 * @param[out] context Optional, stores error information
 *
 * @param[in] uri @parblock
 *   URI of the Nix store, copied.
 *
 *   If `NULL`, the store from the settings will be used.
 *   Note that `"auto"` holds a strange middle ground, reading part of the general environment, but not all of it. It
 * ignores `NIX_REMOTE` and the `store` option. For this reason, `NULL` is most likely the better choice.
 *
 *   For supported store URLs, see [*Store URL format* in the Nix Reference
 * Manual](https://nix.dev/manual/nix/stable/store/types/#store-url-format).
 * @endparblock
 *
 * @param[in] params @parblock
 *   optional, null-terminated array of key-value pairs, e.g. {{"endpoint",
 * "https://s3.local"}}.
 *
 *   See [*Store Types* in the Nix Reference Manual](https://nix.dev/manual/nix/stable/store/types).
 * @endparblock
 *
 * @return a Store pointer, NULL in case of errors
 *
 * @see nix_store_free
 */
Store * nix_store_open(nix_c_context * context, const char * uri, const char *** params);

/**
 * @brief Deallocate a nix store and free any resources if not also held by other Store instances.
 *
 * Does not fail.
 *
 * @param[in] store the store to free
 */
void nix_store_free(Store * store);

/**
 * @brief get the URI of a nix store
 * @param[out] context Optional, stores error information
 * @param[in] store nix store reference
 * @param[in] callback Called with the URI.
 * @param[in] user_data optional, arbitrary data, passed to the callback when it's called.
 * @see nix_get_string_callback
 * @return error code, NIX_OK on success.
 */
nix_err nix_store_get_uri(nix_c_context * context, Store * store, nix_get_string_callback callback, void * user_data);

/**
 * @brief get the storeDir of a Nix store, typically `"/nix/store"`
 * @param[out] context Optional, stores error information
 * @param[in] store nix store reference
 * @param[in] callback Called with the URI.
 * @param[in] user_data optional, arbitrary data, passed to the callback when it's called.
 * @see nix_get_string_callback
 * @return error code, NIX_OK on success.
 */
nix_err
nix_store_get_storedir(nix_c_context * context, Store * store, nix_get_string_callback callback, void * user_data);

/**
 * @brief Parse a Nix store path into a StorePath
 *
 * @note Don't forget to free this path using nix_store_path_free()!
 * @param[out] context Optional, stores error information
 * @param[in] store nix store reference
 * @param[in] path Path string to parse, copied
 * @return owned store path, NULL on error
 */
StorePath * nix_store_parse_path(nix_c_context * context, Store * store, const char * path);

/**
 * @brief Get the path name (e.g. "name" in /nix/store/...-name)
 *
 * @param[in] store_path the path to get the name from
 * @param[in] callback called with the name
 * @param[in] user_data arbitrary data, passed to the callback when it's called.
 */
void nix_store_path_name(const StorePath * store_path, nix_get_string_callback callback, void * user_data);

/**
 * @brief Copy a StorePath
 *
 * @param[in] p the path to copy
 * @return a new StorePath
 */
StorePath * nix_store_path_clone(const StorePath * p);

/** @brief Deallocate a StorePath
 *
 * Does not fail.
 * @param[in] p the path to free
 */
void nix_store_path_free(StorePath * p);

/**
 * @brief Check if a StorePath is valid (i.e. that corresponding store object and its closure of references exists in
 * the store)
 * @param[out] context Optional, stores error information
 * @param[in] store Nix Store reference
 * @param[in] path Path to check
 * @return true or false, error info in context
 */
bool nix_store_is_valid_path(nix_c_context * context, Store * store, StorePath * path);

/**
 * @brief Get the physical location of a store path
 *
 * A store may reside at a different location than its `storeDir` suggests.
 * This situation is called a relocated store.
 * Relocated stores are used during NixOS installation, as well as in restricted computing environments that don't offer
 * a writable `/nix/store`.
 *
 * Not all types of stores support this operation.
 *
 * @param[in] context Optional, stores error information
 * @param[in] store nix store reference
 * @param[in] path the path to get the real path from
 * @param[in] callback called with the real path
 * @param[in] user_data arbitrary data, passed to the callback when it's called.
 */
nix_err nix_store_real_path(
    nix_c_context * context, Store * store, StorePath * path, nix_get_string_callback callback, void * user_data);

// nix_err nix_store_ensure(Store*, const char*);
// nix_err nix_store_build_paths(Store*);
/**
 * @brief Realise a Nix store path
 *
 * Blocking, calls callback once for each realised output.
 *
 * @note When working with expressions, consider using e.g. nix_string_realise to get the output. `.drvPath` may not be
 * accurate or available in the future. See https://github.com/NixOS/nix/issues/6507
 *
 * @param[out] context Optional, stores error information
 * @param[in] store Nix Store reference
 * @param[in] path Path to build
 * @param[in] userdata data to pass to every callback invocation
 * @param[in] callback called for every realised output
 */
nix_err nix_store_realise(
    nix_c_context * context,
    Store * store,
    StorePath * path,
    void * userdata,
    void (*callback)(void * userdata, const char * outname, const char * out));

/**
 * @brief get the version of a nix store.
 *
 * If the store doesn't have a version (like the dummy store), returns an empty string.
 *
 * @param[out] context Optional, stores error information
 * @param[in] store nix store reference
 * @param[in] callback Called with the version.
 * @param[in] user_data optional, arbitrary data, passed to the callback when it's called.
 * @see nix_get_string_callback
 * @return error code, NIX_OK on success.
 */
nix_err
nix_store_get_version(nix_c_context * context, Store * store, nix_get_string_callback callback, void * user_data);

/**
 * @brief Copy the closure of `path` from `srcStore` to `dstStore`.
 *
 * @param[out] context Optional, stores error information
 * @param[in] srcStore nix source store reference
 * @param[in] dstStore nix destination store reference
 * @param[in] path Path to copy
 */
nix_err nix_store_copy_closure(nix_c_context * context, Store * srcStore, Store * dstStore, StorePath * path);

// cffi end
#ifdef __cplusplus
}
#endif
/**
 * @}
 */
#endif // NIX_API_STORE_H
