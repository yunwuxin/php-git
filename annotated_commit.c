#include "php_git2.h"
#include "php_git2_priv.h"
#include "annotated_commit.h"


/* {{{ proto resource git_annotated_commit_from_ref(resource $repo, resource $ref)
 */
PHP_FUNCTION(git_annotated_commit_from_ref)
{
    php_git2_t *result = NULL, *_repo = NULL, *_ref = NULL;
    git_annotated_commit *out = NULL;
    zval *repo = NULL, *ref = NULL;
    int error = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
    "rr", &repo, &ref) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
    ZEND_FETCH_RESOURCE(_ref, php_git2_t*, &ref, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
    error = git_annotated_commit_from_ref(&out, PHP_GIT2_V(_repo, repository), PHP_GIT2_V(_ref, reference));
    if (php_git2_check_error(error, "git_annotated_commit_from_ref" TSRMLS_CC)) {
        RETURN_FALSE;
    }
    if (php_git2_make_resource(&result, PHP_GIT2_TYPE_ANNOTATED_COMMIT, out, 1 TSRMLS_CC)) {
        RETURN_FALSE;
    }
    ZVAL_RESOURCE(return_value, GIT2_RVAL_P(result));
}
/* }}} */

/* {{{ proto resource git_annotated_commit_from_fetchhead(resource $repo, string $branch_name, string $remote_url, string $oid)
 */
PHP_FUNCTION(git_annotated_commit_from_fetchhead)
{
    php_git2_t *result = NULL, *_repo = NULL;
    git_annotated_commit *out = NULL;
    zval *repo = NULL;
    char *branch_name = NULL, *remote_url = NULL, *oid = NULL;
    int branch_name_len = 0, remote_url_len = 0, oid_len = 0, error = 0;
    git_oid __oid = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
    "rsss", &repo, &branch_name, &branch_name_len, &remote_url, &remote_url_len, &oid, &oid_len) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
    if (git_oid_fromstrn(&__oid, oid, oid_len)) {
        RETURN_FALSE;
    }
    error = git_annotated_commit_from_fetchhead(&out, PHP_GIT2_V(_repo, repository), branch_name, remote_url, &__oid);
    if (php_git2_check_error(error, "git_annotated_commit_from_fetchhead" TSRMLS_CC)) {
        RETURN_FALSE;
    }
    if (php_git2_make_resource(&result, PHP_GIT2_TYPE_ANNOTATED_COMMIT, out, 1 TSRMLS_CC)) {
        RETURN_FALSE;
    }
    ZVAL_RESOURCE(return_value, GIT2_RVAL_P(result));
}
/* }}} */

/* {{{ proto resource git_annotated_commit_lookup(resource $repo, string $oid)
 */
PHP_FUNCTION(git_annotated_commit_lookup)
{
    php_git2_t *result = NULL, *_repo = NULL;
    git_annotated_commit *out = NULL;
    zval *repo = NULL;
    char *oid = NULL;
    int oid_len = 0, error = 0;
    git_oid __oid = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
    "rs", &repo, &oid, &oid_len) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(_repo, php_git2_t*, &repo, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
    if (git_oid_fromstrn(&__oid, oid, oid_len)) {
        RETURN_FALSE;
    }
    error = git_annotated_commit_lookup(&out, PHP_GIT2_V(_repo, repository), &__oid);
    if (php_git2_check_error(error, "git_annotated_commit_lookup" TSRMLS_CC)) {
        RETURN_FALSE;
    }
    if (php_git2_make_resource(&result, PHP_GIT2_TYPE_ANNOTATED_COMMIT, out, 1 TSRMLS_CC)) {
        RETURN_FALSE;
    }
    ZVAL_RESOURCE(return_value, GIT2_RVAL_P(result));
}
/* }}} */


/* {{{ proto void git_annotated_commit_free(resource $commit)
 */
PHP_FUNCTION(git_annotated_commit_free)
{
    zval *commit = NULL;
    php_git2_t *_commit = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
    "r", &commit) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(_commit, php_git2_t*, &commit, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
    if (GIT2_SHOULD_FREE(_commit)) {
        git_annotated_commit_free(PHP_GIT2_V(_commit, annotated_commit));
        GIT2_SHOULD_FREE(_commit) = 0;
    };
    zval_ptr_dtor(&commit);
}
/* }}} */