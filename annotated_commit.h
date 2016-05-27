#ifndef PHP_GIT2_ANNOTATED_COMMIT_H
#define PHP_GIT2_ANNOTATED_COMMIT_H

ZEND_BEGIN_ARG_INFO_EX(arginfo_git_annotated_commit_from_ref, 0, 0, 2)
ZEND_ARG_INFO(0, repo)
ZEND_ARG_INFO(0, ref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_git_annotated_commit_from_fetchhead, 0, 0, 4)
ZEND_ARG_INFO(0, repo)
ZEND_ARG_INFO(0, branch_name)
ZEND_ARG_INFO(0, remote_url)
ZEND_ARG_INFO(0, oid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_git_annotated_commit_free, 0, 0, 1)
ZEND_ARG_INFO(0, commit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfogit_annotated_commit_lookup, 0, 0, 2)
ZEND_ARG_INFO(0, repo)
ZEND_ARG_INFO(0, oid)
ZEND_END_ARG_INFO()

/* {{{ proto resource git_annotated_commit_from_ref(resource $repo, resource $ref)
*/
PHP_FUNCTION(git_annotated_commit_from_ref);

/* {{{ proto resource git_annotated_commit_from_fetchhead(resource $repo, string $branch_name, string $remote_url, string $oid)
*/
PHP_FUNCTION(git_annotated_commit_from_fetchhead);

/* {{{ proto resource git_annotated_commit_free(resource $commit)
*/
PHP_FUNCTION(git_annotated_commit_free);

/* {{{ proto resource git_annotated_commit_lookup(resource $repo, string $oid)
*/
PHP_FUNCTION(git_annotated_commit_lookup);

#endif //PHP_GIT2_ANNOTATED_COMMIT_H
