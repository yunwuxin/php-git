#include "php_git2.h"
#include "php_git2_priv.h"
#include "merge.h"

/* {{{ proto resource git_merge_base(resource $repo, string $one, string $two)
 */
PHP_FUNCTION(git_merge_base)
{
	php_git2_t *_repo = NULL;
	git_oid out = {0}, __one = {0}, __two = {0};
	zval *repo = NULL;
	char *one = NULL, *two = NULL, oid[41] = {0};
	int one_len = 0, two_len = 0, error = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"rss", &repo, &one, &one_len, &two, &two_len) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	if (git_oid_fromstrn(&__one, one, one_len)) {
		RETURN_FALSE;
	}
	if (git_oid_fromstrn(&__two, two, two_len)) {
		RETURN_FALSE;
	}
	error = git_merge_base(&out, PHP_GIT2_V(_repo, repository), &__one, &__two);
	if (php_git2_check_error(error, "git_merge_base" TSRMLS_CC)) {
		RETURN_FALSE;
	}
	git_oid_fmt(oid, &out);
	RETURN_STRING(oid, 1);
}
/* }}} */

/* {{{ proto resource git_merge_base_many(resource $repo, long $length, string $input_array[])
 */
PHP_FUNCTION(git_merge_base_many)
{
//	php_git2_t *result = NULL, *_repo = NULL;
//	git_oid out = {0}, __input_array[] = {0};
//	zval *repo = NULL;
//	long length = 0;
//	char *input_array[] = NULL;
//	int input_array[]_len = 0, error = 0;
	
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
//		"rls", &repo, &length, &input_array[], &input_array[]_len) == FAILURE) {
//		return;
//	}
	
//	ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
//	if (git_oid_fromstrn(&__input_array[], input_array[], input_array[]_len)) {
//		RETURN_FALSE;
//	}
//	error = git_merge_base_many(&__out, PHP_GIT2_V(_repo, repository), length, __input_array[]);
//	if (php_git2_check_error(error, "git_merge_base_many" TSRMLS_CC)) {
//		RETURN_FALSE;
//	}
//	if (php_git2_make_resource(&result, PHP_GIT2_TYPE_OID, out, 1 TSRMLS_CC)) {
//		RETURN_FALSE;
//	}
//	ZVAL_RESOURCE(return_value, GIT2_RVAL_P(result));
}
/* }}} */


/* {{{ proto resource git_merge_trees(resource $repo, resource $ancestor_tree, resource $our_tree, resource $their_tree,  $opts)
 */
PHP_FUNCTION(git_merge_trees)
{
	php_git2_t *result = NULL, *_repo = NULL, *_ancestor_tree = NULL, *_our_tree = NULL, *_their_tree = NULL;
	git_index *out = NULL;
	zval *repo = NULL, *ancestor_tree = NULL, *our_tree = NULL, *their_tree = NULL, *opts = NULL;
	int error = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"rrrr<git_merge_tree_opts>", &repo, &ancestor_tree, &our_tree, &their_tree, &opts) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	ZEND_FETCH_RESOURCE(_ancestor_tree, php_git2_t*, &ancestor_tree, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	ZEND_FETCH_RESOURCE(_our_tree, php_git2_t*, &our_tree, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	ZEND_FETCH_RESOURCE(_their_tree, php_git2_t*, &their_tree, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	error = git_merge_trees(&out, PHP_GIT2_V(_repo, repository), PHP_GIT2_V(_ancestor_tree, tree), PHP_GIT2_V(_our_tree, tree), PHP_GIT2_V(_their_tree, tree), opts);
	if (php_git2_check_error(error, "git_merge_trees" TSRMLS_CC)) {
		RETURN_FALSE;
	}
	if (php_git2_make_resource(&result, PHP_GIT2_TYPE_INDEX, out, 1 TSRMLS_CC)) {
		RETURN_FALSE;
	}
	ZVAL_RESOURCE(return_value, GIT2_RVAL_P(result));
}
/* }}} */

/* {{{ proto resource git_merge(resource $repo, array $their_heads, array $opts)
 */
PHP_FUNCTION(git_merge)
{
	php_git2_t *result = NULL, *_repo = NULL, *_their_head = NULL;
	git_merge_result *out = NULL;
	zval *repo = NULL, *opts = NULL, *theirhead = NULL;
	git_annotated_commit *heads[1];
	int error = 0;
	git_merge_options options = GIT_MERGE_OPTIONS_INIT;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"rza", &repo, &theirhead, &opts) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	ZEND_FETCH_RESOURCE(_their_head, php_git2_t*, &theirhead, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	heads[0] = PHP_GIT2_V(_their_head, annotated_commit);

	error = git_merge(&out, PHP_GIT2_V(_repo, repository), heads, 1, &options);
	if (php_git2_check_error(error, "git_merge" TSRMLS_CC)) {
		RETURN_FALSE;
	}
	if (php_git2_make_resource(&result, PHP_GIT2_TYPE_MERGE_RESULT, out, 1 TSRMLS_CC)) {
		RETURN_FALSE;
	}
	ZVAL_RESOURCE(return_value, GIT2_RVAL_P(result));
}
/* }}} */

/* {{{ proto long git_merge_result_is_uptodate(resource $merge_result)
 */
PHP_FUNCTION(git_merge_result_is_uptodate)
{
//	int result = 0;
//	zval *merge_result = NULL;
//	php_git2_t *_merge_result = NULL;
//
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
//		"r", &merge_result) == FAILURE) {
//		return;
//	}
//
//	ZEND_FETCH_RESOURCE(_merge_result, php_git2_t*, &merge_result, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
//	result = git_merge_result_is_uptodate(PHP_GIT2_V(_merge_result, merge_result));
//	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto long git_merge_result_is_fastforward(resource $merge_result)
 */
PHP_FUNCTION(git_merge_result_is_fastforward)
{
//	int result = 0;
//	zval *merge_result = NULL;
//	php_git2_t *_merge_result = NULL;
//
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
//		"r", &merge_result) == FAILURE) {
//		return;
//	}
//
//	ZEND_FETCH_RESOURCE(_merge_result, php_git2_t*, &merge_result, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
//	result = git_merge_result_is_fastforward(PHP_GIT2_V(_merge_result, merge_result));
//	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto resource git_merge_result_fastforward_oid(resource $merge_result)
 */
PHP_FUNCTION(git_merge_result_fastforward_oid)
{
//	php_git2_t *_merge_result = NULL;
//	git_oid out = {0};
//	zval *merge_result = NULL;
//	int error = 0;
//	char buf[41] = {0};
//
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
//		"r", &merge_result) == FAILURE) {
//		return;
//	}
//
//	ZEND_FETCH_RESOURCE(_merge_result, php_git2_t*, &merge_result, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
//	error = git_merge_result_fastforward_oid(&out, PHP_GIT2_V(_merge_result, merge_result));
//	if (php_git2_check_error(error, "git_merge_result_fastforward_oid" TSRMLS_CC)) {
//		RETURN_FALSE;
//	}
//	git_oid_fmt(buf, &out);
//	RETURN_STRING(buf, 1);
}
/* }}} */

/* {{{ proto void git_merge_result_free(resource $merge_result)
 */
PHP_FUNCTION(git_merge_result_free)
{
//	zval *merge_result = NULL;
//	php_git2_t *_merge_result = NULL;
//
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
//		"r", &merge_result) == FAILURE) {
//		return;
//	}
//
//	ZEND_FETCH_RESOURCE(_merge_result, php_git2_t*, &merge_result, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
//	if (GIT2_SHOULD_FREE(_merge_result)) {
//		git_merge_result_free(PHP_GIT2_V(_merge_result, merge_result));
//		GIT2_SHOULD_FREE(_merge_result) = 0;
//	};
//	zval_ptr_dtor(&merge_result);
}
/* }}} */

