#include "php_git2.h"
#include "php_git2_priv.h"
#include "reset.h"

/* {{{ proto long git_reset(resource $repo, resource $target,  $reset_type)
 */
PHP_FUNCTION(git_reset)
{
	int result = 0;
	zval *repo = NULL, *target = NULL;
	php_git2_t *_repo = NULL, *_target = NULL;
	long reset_type = 0;
	git_checkout_options options = GIT_CHECKOUT_OPTIONS_INIT;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"rrl", &repo, &target, &reset_type) == FAILURE) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	ZEND_FETCH_RESOURCE(_target, php_git2_t*, &target, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	result = git_reset(PHP_GIT2_V(_repo, repository), PHP_GIT2_V(_target, object), reset_type, &options);
	RETURN_LONG(result);
}
/* }}} */

/* {{{ proto long git_reset_default(resource $repo, resource $target, array $pathspecs)
 */
PHP_FUNCTION(git_reset_default)
{
	int result = 0;
	zval *repo = NULL, *target = NULL, *pathspecs = NULL, *array = NULL;
	php_git2_t *_repo = NULL, *_target = NULL;
	git_strarray _pathspecs = {0};
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"rra", &repo, &target, &pathspecs) == FAILURE) {
		return;
	}
	if (zend_hash_num_elements(pathspecs) > 0) {
		php_git2_array_to_strarray(&_pathspecs, pathspecs TSRMLS_CC);
	}
	ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	ZEND_FETCH_RESOURCE(_target, php_git2_t*, &target, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	result = git_reset_default(PHP_GIT2_V(_repo, repository), PHP_GIT2_V(_target, object), &_pathspecs);
	if (zend_hash_num_elements(pathspecs) > 0) {
		git_strarray_free(&array);
	}

	RETURN_LONG(result);
}
/* }}} */

