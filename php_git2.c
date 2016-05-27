/*
 * PHP Libgit2 Extension
 *
 * https://github.com/libgit2/php-git
 *
 * Copyright 2014 Shuhei Tanuma.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "php_git2.h"
#include "php_git2_priv.h"
#include "repository.h"
#include "commit.h"
#include "tree.h"
#include "clone.h"
#include "blob.h"
#include "revwalk.h"
#include "treebuilder.h"
#include "reference.h"
#include "g_config.h"
#include "object.h"
#include "index.h"
#include "revparse.h"
#include "branch.h"
#include "tag.h"
#include "status.h"
#include "cred.h"
#include "remote.h"
#include "transport.h"
#include "diff.h"
#include "checkout.h"
#include "filter.h"
#include "ignore.h"
#include "indexer.h"
#include "pathspec.h"
#include "patch.h"
#include "annotated_commit.h"
#include "merge.h"
#include "note.h"
#include "odb.h"
#include "reflog.h"
#include "blame.h"
#include "packbuilder.h"
#include "stash.h"
#include "signature.h"
#include "reset.h"
#include "message.h"
#include "submodule.h"
#include "attr.h"
#include "giterr.h"
#include "push.h"
#include "refspec.h"
#include "graph.h"
#include "blame.h"

int git2_resource_handle;

zend_class_entry *php_git2_odb_backend_foreach_callback_class_entry;

void static destruct_git2(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_git2_t *resource = (php_git2_t *)rsrc->ptr;
	if (resource->should_free_v) {
		switch (resource->type) {
			case PHP_GIT2_TYPE_REPOSITORY:
				//git_repository_free(PHP_GIT2_V(resource, repository));
				break;
			case PHP_GIT2_TYPE_COMMIT:
				//git_commit_free(PHP_GIT2_V(resource, commit));
				break;
			case PHP_GIT2_TYPE_TREE:
				//git_tree_free(PHP_GIT2_V(resource, tree));
				break;
			case PHP_GIT2_TYPE_TREE_ENTRY:
				//git_tree_entry_free(PHP_GIT2_V(resource, tree_entry));
				break;
			case PHP_GIT2_TYPE_BLOB:
				//git_blob_free(PHP_GIT2_V(resource, blob));
				break;
			case PHP_GIT2_TYPE_REVWALK:
				git_revwalk_free(PHP_GIT2_V(resource, revwalk));
				break;
			case PHP_GIT2_TYPE_TREEBUILDER:
				//git_treebuilder_free(PHP_GIT2_V(resource, treebuilder));
				break;
			case PHP_GIT2_TYPE_REFERENCE:
				//git_reference_free(PHP_GIT2_V(resource, reference));
				break;
			case PHP_GIT2_TYPE_CONFIG:
				git_config_free(PHP_GIT2_V(resource, config));
				break;
			case PHP_GIT2_TYPE_OBJECT:
				git_object_free(PHP_GIT2_V(resource, object));
				break;
			case PHP_GIT2_TYPE_FILTER:
			{
				php_git2_filter *filter = (php_git2_filter*)PHP_GIT2_V(resource, filter);
				zval_ptr_dtor(&filter->multi->payload);
				php_git2_multi_cb_free(filter->multi);
				efree(filter);
				break;
			}
			case PHP_GIT2_TYPE_FILTER_LIST:
				git_filter_list_free(PHP_GIT2_V(resource, filter_list));
				break;
			case PHP_GIT2_TYPE_ODB_BACKEND:
			{
				php_git2_odb_backend *backend = (php_git2_odb_backend*)PHP_GIT2_V(resource, odb_backend);
				zval_ptr_dtor(&backend->multi->payload);
				php_git2_multi_cb_free(backend->multi);
				efree(PHP_GIT2_V(resource, odb_backend));
				break;
			}
		}
	}

	efree(resource);
}

ZEND_DECLARE_MODULE_GLOBALS(git2);

static zend_class_entry *php_git2_get_exception_base(TSRMLS_D)
{
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
	return zend_exception_get_default();
#else
	return zend_exception_get_default(TSRMLS_C);
#endif
}

int php_git2_make_resource(php_git2_t **out, enum php_git2_resource_type type, void *resource, int should_free TSRMLS_DC)
{
	php_git2_t *result = NULL;

	PHP_GIT2_MAKE_RESOURCE_NOCHECK(result);
	if (result == NULL) {
		return 1;
	}

	switch (type) {
		case PHP_GIT2_TYPE_REPOSITORY:
			PHP_GIT2_V(result, repository) = (git_repository*)resource;
			break;
		case PHP_GIT2_TYPE_COMMIT:
			PHP_GIT2_V(result, commit) = (git_commit*)resource;
			break;
		case PHP_GIT2_TYPE_TREE:
			PHP_GIT2_V(result, tree) = (git_tree*)resource;
			break;
		case PHP_GIT2_TYPE_TREE_ENTRY:
			PHP_GIT2_V(result, tree_entry) = (git_tree_entry*)resource;
			break;
		case PHP_GIT2_TYPE_BLOB:
			PHP_GIT2_V(result, blob) = (git_blob*)resource;
			break;
		case PHP_GIT2_TYPE_REVWALK:
			PHP_GIT2_V(result, revwalk) = (git_revwalk*)resource;
			break;
		case PHP_GIT2_TYPE_TREEBUILDER:
			PHP_GIT2_V(result, treebuilder) = (git_treebuilder*)resource;
			break;
		case PHP_GIT2_TYPE_REFERENCE:
			PHP_GIT2_V(result, reference) = (git_reference*)resource;
			break;
		case PHP_GIT2_TYPE_REFERENCE_ITERATOR:
			PHP_GIT2_V(result, reference_iterator) = (git_reference_iterator*)resource;
			break;
		case PHP_GIT2_TYPE_CONFIG:
			PHP_GIT2_V(result, config) = (git_config*)resource;
			break;
		case PHP_GIT2_TYPE_CONFIG_ITERATOR:
			PHP_GIT2_V(result, config_iterator) = (git_config_iterator*)resource;
			break;
		case PHP_GIT2_TYPE_OBJECT:
			PHP_GIT2_V(result, object) = (git_object*)resource;
			break;
		case PHP_GIT2_TYPE_INDEX:
			PHP_GIT2_V(result, index) = (git_index*)resource;
			break;
		case PHP_GIT2_TYPE_ODB:
			PHP_GIT2_V(result, odb) = (git_odb*)resource;
			break;
		case PHP_GIT2_TYPE_REFDB:
			PHP_GIT2_V(result, refdb) = (git_refdb*)resource;
			break;
		case PHP_GIT2_TYPE_STATUS_LIST:
			PHP_GIT2_V(result, status_list) = (git_status_list*)resource;
			break;
		case PHP_GIT2_TYPE_BRANCH_ITERATOR:
			PHP_GIT2_V(result, branch_iterator) = (git_branch_iterator*)resource;
			break;
		case PHP_GIT2_TYPE_TAG:
			PHP_GIT2_V(result, tag) = (git_tag*)resource;
			break;
		case PHP_GIT2_TYPE_CRED:
			PHP_GIT2_V(result, cred) = (git_cred*)resource;
			break;
		case PHP_GIT2_TYPE_TRANSPORT:
			PHP_GIT2_V(result, transport) = (git_transport*)resource;
			break;
		case PHP_GIT2_TYPE_REMOTE:
			PHP_GIT2_V(result, remote) = (git_remote*)resource;
			break;
		case PHP_GIT2_TYPE_DIFF:
			PHP_GIT2_V(result, diff) = (git_diff*)resource;
			break;
		case PHP_GIT2_TYPE_MERGE_RESULT:
			PHP_GIT2_V(result, merge_result) = (git_merge_result*)resource;
			break;
		case PHP_GIT2_TYPE_ANNOTATED_COMMIT:
			PHP_GIT2_V(result, annotated_commit) = (git_annotated_commit*)resource;
			break;
		case PHP_GIT2_TYPE_PATHSPEC:
			PHP_GIT2_V(result, pathspec) = (git_pathspec*)resource;
			break;
		case PHP_GIT2_TYPE_PATHSPEC_MATCH_LIST:
			PHP_GIT2_V(result, pathspec_match_list) = (git_pathspec_match_list*)resource;
			break;
		case PHP_GIT2_TYPE_PATCH:
			PHP_GIT2_V(result, patch) = (git_patch*)resource;
			break;
		case PHP_GIT2_TYPE_DIFF_HUNK:
			PHP_GIT2_V(result, diff_hunk) = (git_diff_hunk*)resource;
			break;
		case PHP_GIT2_TYPE_BUF:
			PHP_GIT2_V(result, buf) = (git_buf*)resource;
			break;
		case PHP_GIT2_TYPE_FILTER_LIST:
			PHP_GIT2_V(result, filter_list) = (git_filter_list*)resource;
			break;
		case PHP_GIT2_TYPE_FILTER_SOURCE:
			PHP_GIT2_V(result, filter_source) = (git_filter_source*)resource;
			break;
		case PHP_GIT2_TYPE_DIFF_LINE:
			PHP_GIT2_V(result, diff_line) = (git_diff_line*)resource;
			break;
		case PHP_GIT2_TYPE_INDEX_CONFLICT_ITERATOR:
			PHP_GIT2_V(result, index_conflict_iterator) = (git_index_conflict_iterator*)resource;
			break;
		case PHP_GIT2_TYPE_SMART_SUBTRANSPORT:
			PHP_GIT2_V(result, smart_subtransport) = (git_smart_subtransport*)resource;
			break;
		case PHP_GIT2_TYPE_NOTE:
			PHP_GIT2_V(result, note) = (git_note*)resource;
			break;
		case PHP_GIT2_TYPE_NOTE_ITERATOR:
			PHP_GIT2_V(result, note_iterator) = (git_note_iterator*)resource;
			break;
		case PHP_GIT2_TYPE_ODB_STREAM:
			PHP_GIT2_V(result, odb_stream) = (git_odb_stream*)resource;
			break;
		case PHP_GIT2_TYPE_ODB_OBJECT:
			PHP_GIT2_V(result, odb_object) = (git_odb_object*)resource;
			break;
		case PHP_GIT2_TYPE_ODB_WRITEPACK:
			PHP_GIT2_V(result, odb_writepack) = (git_odb_writepack*)resource;
			break;
		case PHP_GIT2_TYPE_ODB_BACKEND:
			PHP_GIT2_V(result, odb_backend) = (git_odb_backend*)resource;
			break;
		case PHP_GIT2_TYPE_REFLOG:
			PHP_GIT2_V(result, reflog) = (git_reflog*)resource;
			break;
		case PHP_GIT2_TYPE_REFLOG_ENTRY:
			PHP_GIT2_V(result, reflog_entry) = (git_reflog_entry*)resource;
			break;
		case PHP_GIT2_TYPE_BLAME:
			PHP_GIT2_V(result, blame) = (git_blame*)resource;
			break;
		case PHP_GIT2_TYPE_PACKBUILDER:
			PHP_GIT2_V(result, packbuilder) = (git_packbuilder*)resource;
			break;
		case PHP_GIT2_TYPE_SUBMODULE:
			PHP_GIT2_V(result, submodule) = (git_submodule*)resource;
			break;
		case PHP_GIT2_TYPE_PUSH:
			PHP_GIT2_V(result, push) = (git_push*)resource;
			break;
		case PHP_GIT2_TYPE_FILTER:
			PHP_GIT2_V(result, filter) = (git_filter*)resource;
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "passed resource type does not support. probably bug.");
	}

	result->type = type;
	result->resource_id = PHP_GIT2_LIST_INSERT(result, git2_resource_handle);
	result->should_free_v = should_free;

	*out = result;
	return 0;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_git_resource_type, 0, 0, 1)
	ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

/* {{{ proto long git_resource_type(resource $git)
 */
PHP_FUNCTION(git_resource_type)
{
	zval *resource = NULL;
	php_git2_t *_resource= NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"r", &resource) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(_resource, php_git2_t*, &resource, -1, PHP_GIT2_RESOURCE_NAME, git2_resource_handle);
	RETURN_LONG(_resource->type);
}
/* }}} */

PHP_FUNCTION(git_checkout_opts_new)
{
	zval *tmp;
	git_checkout_options opt = GIT_CHECKOUT_OPTIONS_INIT;

	php_git2_git_checkout_opts_to_array(&opt, &tmp TSRMLS_CC);
	RETURN_ZVAL(tmp, 0, 1);
}

PHP_FUNCTION(git_libgit2_capabilities)
{
	//RETURN_LONG(git_libgit2_capabilities());
}


PHP_FUNCTION(git_libgit2_version)
{
	char buf[32] = {0};
	int major, minor, rev;

	git_libgit2_version(&major, &minor, &rev);
	snprintf(buf, 32, "%d.%d.%d", major, minor, rev);

	RETURN_STRING(buf, 1);
}

static zend_function_entry php_git2_functions[] = {
	/* repository */
	PHP_FE(git_repository_new, arginfo_git_repository_new)
	PHP_FE(git_repository_init, arginfo_git_repository_init)
	PHP_FE(git_repository_open_bare, arginfo_git_repository_open_bare)
	PHP_FE(git_repository_open, arginfo_git_repository_open)
	PHP_FE(git_repository_get_namespace, arginfo_git_repository_get_namespace)
	PHP_FE(git_repository_workdir, arginfo_git_repository_workdir)
	PHP_FE(git_repository_wrap_odb, arginfo_git_repository_wrap_odb)
	PHP_FE(git_repository_discover, arginfo_git_repository_discover)
	PHP_FE(git_repository_open_ext, arginfo_git_repository_open_ext)
	PHP_FE(git_repository_free, arginfo_git_repository_free)
	PHP_FE(git_repository_init_ext, arginfo_git_repository_init_ext)
	PHP_FE(git_repository_head, arginfo_git_repository_head)
	PHP_FE(git_repository_head_detached, arginfo_git_repository_head_detached)
	PHP_FE(git_repository_head_unborn, arginfo_git_repository_head_unborn)
	PHP_FE(git_repository_is_empty, arginfo_git_repository_is_empty)
	PHP_FE(git_repository_path, arginfo_git_repository_path)
	PHP_FE(git_repository_set_workdir, arginfo_git_repository_set_workdir)
	PHP_FE(git_repository_is_bare, arginfo_git_repository_is_bare)
	PHP_FE(git_repository_config, arginfo_git_repository_config)
	PHP_FE(git_repository_odb, arginfo_git_repository_odb)
	PHP_FE(git_repository_refdb, arginfo_git_repository_refdb)
	PHP_FE(git_repository_index, arginfo_git_repository_index)
	PHP_FE(git_repository_message, arginfo_git_repository_message)
	PHP_FE(git_repository_message_remove, arginfo_git_repository_message_remove)
	PHP_FE(git_repository_merge_cleanup, arginfo_git_repository_merge_cleanup)
	PHP_FE(git_repository_fetchhead_foreach, arginfo_git_repository_fetchhead_foreach)
	PHP_FE(git_repository_mergehead_foreach, arginfo_git_repository_mergehead_foreach)
	PHP_FE(git_repository_hashfile, arginfo_git_repository_hashfile)
	PHP_FE(git_repository_set_head, arginfo_git_repository_set_head)
	PHP_FE(git_repository_set_head_detached, arginfo_git_repository_set_head_detached)
	PHP_FE(git_repository_detach_head, arginfo_git_repository_detach_head)
	PHP_FE(git_repository_state, arginfo_git_repository_state)
	PHP_FE(git_repository_set_namespace, arginfo_git_repository_set_namespace)
	PHP_FE(git_repository_is_shallow, arginfo_git_repository_is_shallow)
	PHP_FE(git_repository_init_options_new, NULL)

	/* index */
	PHP_FE(git_index_open, arginfo_git_index_open)
	PHP_FE(git_index_new, arginfo_git_index_new)
	PHP_FE(git_index_free, arginfo_git_index_free)
	PHP_FE(git_index_owner, arginfo_git_index_owner)
	PHP_FE(git_index_caps, arginfo_git_index_caps)
	PHP_FE(git_index_set_caps, arginfo_git_index_set_caps)
	PHP_FE(git_index_read, arginfo_git_index_read)
	PHP_FE(git_index_write, arginfo_git_index_write)
	PHP_FE(git_index_path, arginfo_git_index_path)
	PHP_FE(git_index_read_tree, arginfo_git_index_read_tree)
	PHP_FE(git_index_write_tree, arginfo_git_index_write_tree)
	PHP_FE(git_index_write_tree_to, arginfo_git_index_write_tree_to)
	PHP_FE(git_index_entrycount, arginfo_git_index_entrycount)
	PHP_FE(git_index_clear, arginfo_git_index_clear)
	PHP_FE(git_index_get_byindex, arginfo_git_index_get_byindex)
	PHP_FE(git_index_get_bypath, arginfo_git_index_get_bypath)
	PHP_FE(git_index_remove, arginfo_git_index_remove)
	PHP_FE(git_index_remove_directory, arginfo_git_index_remove_directory)
	PHP_FE(git_index_add, arginfo_git_index_add)
	PHP_FE(git_index_entry_stage, arginfo_git_index_entry_stage)
	PHP_FE(git_index_add_bypath, arginfo_git_index_add_bypath)
	PHP_FE(git_index_remove_bypath, arginfo_git_index_remove_bypath)
	PHP_FE(git_index_add_all, arginfo_git_index_add_all)
	PHP_FE(git_index_remove_all, arginfo_git_index_remove_all)
	PHP_FE(git_index_update_all, arginfo_git_index_update_all)
	PHP_FE(git_index_find, arginfo_git_index_find)
	PHP_FE(git_index_conflict_add, arginfo_git_index_conflict_add)
	PHP_FE(git_index_conflict_get, arginfo_git_index_conflict_get)
	PHP_FE(git_index_conflict_remove, arginfo_git_index_conflict_remove)
	PHP_FE(git_index_conflict_cleanup, arginfo_git_index_conflict_cleanup)
	PHP_FE(git_index_has_conflicts, arginfo_git_index_has_conflicts)
	PHP_FE(git_index_conflict_iterator_new, arginfo_git_index_conflict_iterator_new)
	PHP_FE(git_index_conflict_next, arginfo_git_index_conflict_next)
	PHP_FE(git_index_conflict_iterator_free, arginfo_git_index_conflict_iterator_free)

	/* object */
	PHP_FE(git_object_lookup, arginfo_git_object_lookup)
	PHP_FE(git_object_lookup_prefix, arginfo_git_object_lookup_prefix)
	PHP_FE(git_object_lookup_bypath, arginfo_git_object_lookup_bypath)
	PHP_FE(git_object_id, arginfo_git_object_id)
	PHP_FE(git_object_type, arginfo_git_object_type)
	PHP_FE(git_object_owner, arginfo_git_object_owner)
	PHP_FE(git_object_free, arginfo_git_object_free)
	PHP_FE(git_object_type2string, arginfo_git_object_type2string)
	PHP_FE(git_object_string2type, arginfo_git_object_string2type)
	PHP_FE(git_object_typeisloose, arginfo_git_object_typeisloose)
	PHP_FE(git_object__size, arginfo_git_object__size)
	PHP_FE(git_object_peel, arginfo_git_object_peel)
	PHP_FE(git_object_dup, arginfo_git_object_dup)

	/* clone */
	PHP_FE(git_clone, arginfo_git_clone)

	/* reference */
	PHP_FE(git_reference_lookup, arginfo_git_reference_lookup)
	PHP_FE(git_reference_name_to_id, arginfo_git_reference_name_to_id)
	PHP_FE(git_reference_dwim, arginfo_git_reference_dwim)
	PHP_FE(git_reference_symbolic_create, arginfo_git_reference_symbolic_create)
	PHP_FE(git_reference_create, arginfo_git_reference_create)
	PHP_FE(git_reference_target, arginfo_git_reference_target)
	PHP_FE(git_reference_target_peel, arginfo_git_reference_target_peel)
	PHP_FE(git_reference_symbolic_target, arginfo_git_reference_symbolic_target)
	PHP_FE(git_reference_type, arginfo_git_reference_type)
	PHP_FE(git_reference_name, arginfo_git_reference_name)
	PHP_FE(git_reference_resolve, arginfo_git_reference_resolve)
	PHP_FE(git_reference_owner, arginfo_git_reference_owner)
	PHP_FE(git_reference_symbolic_set_target, arginfo_git_reference_symbolic_set_target)
	PHP_FE(git_reference_set_target, arginfo_git_reference_set_target)
	PHP_FE(git_reference_rename, arginfo_git_reference_rename)
	PHP_FE(git_reference_delete, arginfo_git_reference_delete)
	PHP_FE(git_reference_list, arginfo_git_reference_list)
	PHP_FE(git_reference_foreach, arginfo_git_reference_foreach)
	PHP_FE(git_reference_foreach_name, arginfo_git_reference_foreach_name)
	PHP_FE(git_reference_free, arginfo_git_reference_free)
	PHP_FE(git_reference_cmp, arginfo_git_reference_cmp)
	PHP_FE(git_reference_iterator_new, arginfo_git_reference_iterator_new)
	PHP_FE(git_reference_iterator_glob_new, arginfo_git_reference_iterator_glob_new)
	PHP_FE(git_reference_next, arginfo_git_reference_next)
	PHP_FE(git_reference_next_name, arginfo_git_reference_next_name)
	PHP_FE(git_reference_iterator_free, arginfo_git_reference_iterator_free)
	PHP_FE(git_reference_foreach_glob, arginfo_git_reference_foreach_glob)
	PHP_FE(git_reference_has_log, arginfo_git_reference_has_log)
	PHP_FE(git_reference_is_branch, arginfo_git_reference_is_branch)
	PHP_FE(git_reference_is_remote, arginfo_git_reference_is_remote)
	PHP_FE(git_reference_is_tag, arginfo_git_reference_is_tag)
	PHP_FE(git_reference_normalize_name, arginfo_git_reference_normalize_name)
	PHP_FE(git_reference_peel, arginfo_git_reference_peel)
	PHP_FE(git_reference_is_valid_name, arginfo_git_reference_is_valid_name)
	PHP_FE(git_reference_shorthand, arginfo_git_reference_shorthand)

	/* commit */
	PHP_FE(git_commit_lookup, arginfo_git_commit_lookup)
	PHP_FE(git_commit_author, arginfo_git_commit_author)
	PHP_FE(git_commit_tree, arginfo_git_commit_tree)
	PHP_FE(git_commit_lookup_prefix, arginfo_git_commit_lookup_prefix)
	PHP_FE(git_commit_id, arginfo_git_commit_id)
	PHP_FE(git_commit_owner, arginfo_git_commit_owner)
	PHP_FE(git_commit_message_encoding, arginfo_git_commit_message_encoding)
	PHP_FE(git_commit_message, arginfo_git_commit_message)
	PHP_FE(git_commit_message_raw, arginfo_git_commit_message_raw)
	PHP_FE(git_commit_time, arginfo_git_commit_time)
	PHP_FE(git_commit_time_offset, arginfo_git_commit_time_offset)
	PHP_FE(git_commit_committer, arginfo_git_commit_committer)
	PHP_FE(git_commit_raw_header, arginfo_git_commit_raw_header)
	PHP_FE(git_commit_tree_id, arginfo_git_commit_tree_id)
	PHP_FE(git_commit_parentcount, arginfo_git_commit_parentcount)
	PHP_FE(git_commit_parent, arginfo_git_commit_parent)
	PHP_FE(git_commit_parent_id, arginfo_git_commit_parent_id)
	PHP_FE(git_commit_nth_gen_ancestor, arginfo_git_commit_nth_gen_ancestor)
	PHP_FE(git_commit_create, arginfo_git_commit_create)
	PHP_FE(git_commit_free, arginfo_git_commit_free)

	/* tree */
	PHP_FE(git_tree_free, arginfo_git_tree_free)
	PHP_FE(git_tree_id, arginfo_git_tree_id)
	PHP_FE(git_tree_lookup, arginfo_git_tree_lookup)
	PHP_FE(git_tree_owner, arginfo_git_tree_owner)
	PHP_FE(git_tree_walk, arginfo_git_tree_walk)

	PHP_FE(git_tree_entry_byoid, arginfo_git_tree_entry_byoid)
	PHP_FE(git_tree_entry_byindex, arginfo_git_tree_entry_byindex)
	PHP_FE(git_tree_entry_byname, arginfo_git_tree_entry_byname)
	PHP_FE(git_tree_entry_bypath, arginfo_git_tree_entry_bypath)
	PHP_FE(git_tree_entry_id, arginfo_git_tree_entry_id)
	PHP_FE(git_tree_entry_name, arginfo_git_tree_entry_name)
	PHP_FE(git_tree_entry_type, arginfo_git_tree_entry_type)
	PHP_FE(git_tree_entrycount, arginfo_git_tree_entrycount)
	PHP_FE(git_tree_entry_filemode, arginfo_git_tree_entry_filemode)
	PHP_FE(git_tree_entry_filemode_raw, arginfo_git_tree_entry_filemode_raw)
	PHP_FE(git_tree_entry_cmp, arginfo_git_tree_entry_cmp)
	PHP_FE(git_tree_entry_free, arginfo_git_tree_entry_free)
	PHP_FE(git_tree_entry_dup, arginfo_git_tree_entry_dup)

	/* treebuilder */
	PHP_FE(git_treebuilder_new, arginfo_git_treebuilder_new)
	PHP_FE(git_treebuilder_clear, arginfo_git_treebuilder_clear)
	PHP_FE(git_treebuilder_entrycount, arginfo_git_treebuilder_entrycount)
	PHP_FE(git_treebuilder_free, arginfo_git_treebuilder_free)
	PHP_FE(git_treebuilder_get, arginfo_git_treebuilder_get)
	PHP_FE(git_treebuilder_insert, arginfo_git_treebuilder_insert)
	PHP_FE(git_treebuilder_remove, arginfo_git_treebuilder_remove)
	PHP_FE(git_treebuilder_filter, arginfo_git_treebuilder_filter)
	PHP_FE(git_treebuilder_write, arginfo_git_treebuilder_write)

	/* blob */
	PHP_FE(git_blob_create_frombuffer, arginfo_git_blob_create_frombuffer)
	PHP_FE(git_blob_create_fromchunks, arginfo_git_blob_create_fromchunks)
	PHP_FE(git_blob_create_fromdisk, arginfo_git_blob_create_fromdisk)
	PHP_FE(git_blob_create_fromworkdir, arginfo_git_blob_create_fromworkdir)
	PHP_FE(git_blob_filtered_content, arginfo_git_blob_filtered_content)
	PHP_FE(git_blob_free, arginfo_git_blob_free)
	PHP_FE(git_blob_id, arginfo_git_blob_id)
	PHP_FE(git_blob_is_binary, arginfo_git_blob_is_binary)
	PHP_FE(git_blob_lookup, arginfo_git_blob_lookup)
	PHP_FE(git_blob_lookup_prefix, arginfo_git_blob_lookup_prefix)
	PHP_FE(git_blob_owner, arginfo_git_blob_owner)
	PHP_FE(git_blob_rawcontent, arginfo_git_blob_rawcontent)
	PHP_FE(git_blob_rawsize, arginfo_git_blob_rawsize)

	/* revwalk */
	PHP_FE(git_revwalk_new, arginfo_git_revwalk_new)
	PHP_FE(git_revwalk_reset, arginfo_git_revwalk_reset)
	PHP_FE(git_revwalk_push, arginfo_git_revwalk_push)
	PHP_FE(git_revwalk_push_glob, arginfo_git_revwalk_push_glob)
	PHP_FE(git_revwalk_push_head, arginfo_git_revwalk_push_head)
	PHP_FE(git_revwalk_hide, arginfo_git_revwalk_hide)
	PHP_FE(git_revwalk_hide_glob, arginfo_git_revwalk_hide_glob)
	PHP_FE(git_revwalk_hide_head, arginfo_git_revwalk_hide_head)
	PHP_FE(git_revwalk_push_ref, arginfo_git_revwalk_push_ref)
	PHP_FE(git_revwalk_hide_ref, arginfo_git_revwalk_hide_ref)
	PHP_FE(git_revwalk_next, arginfo_git_revwalk_next)
	PHP_FE(git_revwalk_sorting, arginfo_git_revwalk_sorting)
	PHP_FE(git_revwalk_push_range, arginfo_git_revwalk_push_range)
	PHP_FE(git_revwalk_simplify_first_parent, arginfo_git_revwalk_simplify_first_parent)
	PHP_FE(git_revwalk_free, arginfo_git_revwalk_free)
	PHP_FE(git_revwalk_repository, arginfo_git_revwalk_repository)

	/* config */
	PHP_FE(git_config_find_global, arginfo_git_config_find_global)
	PHP_FE(git_config_find_xdg, arginfo_git_config_find_xdg)
	PHP_FE(git_config_find_system, arginfo_git_config_find_system)
	PHP_FE(git_config_open_default, arginfo_git_config_open_default)
	PHP_FE(git_config_new, arginfo_git_config_new)
	PHP_FE(git_config_add_file_ondisk, arginfo_git_config_add_file_ondisk)
	PHP_FE(git_config_open_ondisk, arginfo_git_config_open_ondisk)
	PHP_FE(git_config_open_level, arginfo_git_config_open_level)
	PHP_FE(git_config_open_global, arginfo_git_config_open_global)
	PHP_FE(git_config_refresh, arginfo_git_config_refresh)
	PHP_FE(git_config_free, arginfo_git_config_free)
	PHP_FE(git_config_get_entry, arginfo_git_config_get_entry)
	PHP_FE(git_config_get_int32, arginfo_git_config_get_int32)
	PHP_FE(git_config_get_int64, arginfo_git_config_get_int64)
	PHP_FE(git_config_get_bool, arginfo_git_config_get_bool)
	PHP_FE(git_config_get_string, arginfo_git_config_get_string)
	PHP_FE(git_config_get_multivar_foreach, arginfo_git_config_get_multivar_foreach)
	PHP_FE(git_config_multivar_iterator_new, arginfo_git_config_multivar_iterator_new)
	PHP_FE(git_config_next, arginfo_git_config_next)
	PHP_FE(git_config_iterator_free, arginfo_git_config_iterator_free)
	PHP_FE(git_config_set_int32, arginfo_git_config_set_int32)
	PHP_FE(git_config_set_int64, arginfo_git_config_set_int64)
	PHP_FE(git_config_set_bool, arginfo_git_config_set_bool)
	PHP_FE(git_config_set_string, arginfo_git_config_set_string)
	PHP_FE(git_config_set_multivar, arginfo_git_config_set_multivar)
	PHP_FE(git_config_delete_entry, arginfo_git_config_delete_entry)
	PHP_FE(git_config_delete_multivar, arginfo_git_config_delete_multivar)
	PHP_FE(git_config_foreach, arginfo_git_config_foreach)
	PHP_FE(git_config_iterator_new, arginfo_git_config_iterator_new)
	PHP_FE(git_config_iterator_glob_new, arginfo_git_config_iterator_glob_new)
	PHP_FE(git_config_foreach_match, arginfo_git_config_foreach_match)
	PHP_FE(git_config_get_mapped, arginfo_git_config_get_mapped)
	PHP_FE(git_config_lookup_map_value, arginfo_git_config_lookup_map_value)
	PHP_FE(git_config_parse_bool, arginfo_git_config_parse_bool)
	PHP_FE(git_config_parse_int32, arginfo_git_config_parse_int32)
	PHP_FE(git_config_parse_int64, arginfo_git_config_parse_int64)
	PHP_FE(git_config_backend_foreach_match, arginfo_git_config_backend_foreach_match)

	/* revparse */
	PHP_FE(git_revparse_single, arginfo_git_revparse_single)
	PHP_FE(git_revparse_ext, arginfo_git_revparse_ext)
	PHP_FE(git_revparse, arginfo_git_revparse)

	/* remote */
	PHP_FE(git_remote_create, arginfo_git_remote_create)
	PHP_FE(git_remote_create_with_fetchspec, arginfo_git_remote_create_with_fetchspec)
	PHP_FE(git_remote_create_inmemory, arginfo_git_remote_create_inmemory)
	PHP_FE(git_remote_load, arginfo_git_remote_load)
	PHP_FE(git_remote_save, arginfo_git_remote_save)
	PHP_FE(git_remote_owner, arginfo_git_remote_owner)
	PHP_FE(git_remote_name, arginfo_git_remote_name)
	PHP_FE(git_remote_url, arginfo_git_remote_url)
	PHP_FE(git_remote_pushurl, arginfo_git_remote_pushurl)
	PHP_FE(git_remote_set_url, arginfo_git_remote_set_url)
	PHP_FE(git_remote_set_pushurl, arginfo_git_remote_set_pushurl)
	PHP_FE(git_remote_add_fetch, arginfo_git_remote_add_fetch)
	PHP_FE(git_remote_get_fetch_refspecs, arginfo_git_remote_get_fetch_refspecs)
	PHP_FE(git_remote_set_fetch_refspecs, arginfo_git_remote_set_fetch_refspecs)
	PHP_FE(git_remote_add_push, arginfo_git_remote_add_push)
	PHP_FE(git_remote_get_push_refspecs, arginfo_git_remote_get_push_refspecs)
	PHP_FE(git_remote_set_push_refspecs, arginfo_git_remote_set_push_refspecs)
	PHP_FE(git_remote_clear_refspecs, arginfo_git_remote_clear_refspecs)
	PHP_FE(git_remote_refspec_count, arginfo_git_remote_refspec_count)
	PHP_FE(git_remote_get_refspec, arginfo_git_remote_get_refspec)
	PHP_FE(git_remote_connect, arginfo_git_remote_connect)
	PHP_FE(git_remote_ls, arginfo_git_remote_ls)
	PHP_FE(git_remote_download, arginfo_git_remote_download)
	PHP_FE(git_remote_connected, arginfo_git_remote_connected)
	PHP_FE(git_remote_stop, arginfo_git_remote_stop)
	PHP_FE(git_remote_disconnect, arginfo_git_remote_disconnect)
	PHP_FE(git_remote_free, arginfo_git_remote_free)
	PHP_FE(git_remote_update_tips, arginfo_git_remote_update_tips)
	PHP_FE(git_remote_fetch, arginfo_git_remote_fetch)
	PHP_FE(git_remote_valid_url, arginfo_git_remote_valid_url)
	PHP_FE(git_remote_supported_url, arginfo_git_remote_supported_url)
	PHP_FE(git_remote_list, arginfo_git_remote_list)
	PHP_FE(git_remote_check_cert, arginfo_git_remote_check_cert)
	PHP_FE(git_remote_set_transport, arginfo_git_remote_set_transport)
	PHP_FE(git_remote_set_callbacks, arginfo_git_remote_set_callbacks)
	PHP_FE(git_remote_stats, arginfo_git_remote_stats)
	PHP_FE(git_remote_autotag, arginfo_git_remote_autotag)
	PHP_FE(git_remote_set_autotag, arginfo_git_remote_set_autotag)
	PHP_FE(git_remote_rename, arginfo_git_remote_rename)
	PHP_FE(git_remote_update_fetchhead, arginfo_git_remote_update_fetchhead)
	PHP_FE(git_remote_set_update_fetchhead, arginfo_git_remote_set_update_fetchhead)
	PHP_FE(git_remote_is_valid_name, arginfo_git_remote_is_valid_name)

	/* cred */
	PHP_FE(git_cred_has_username, arginfo_git_cred_has_username)
	PHP_FE(git_cred_userpass_plaintext_new, arginfo_git_cred_userpass_plaintext_new)
	PHP_FE(git_cred_ssh_key_new, arginfo_git_cred_ssh_key_new)
	PHP_FE(git_cred_ssh_custom_new, arginfo_git_cred_ssh_custom_new)
	PHP_FE(git_cred_default_new, arginfo_git_cred_default_new)
	PHP_FE(git_cred_userpass, arginfo_git_cred_userpass)

	/* status */
	PHP_FE(git_status_foreach, arginfo_git_status_foreach)
	PHP_FE(git_status_foreach_ext, arginfo_git_status_foreach_ext)
	PHP_FE(git_status_file, arginfo_git_status_file)
	PHP_FE(git_status_list_new, arginfo_git_status_list_new)
	PHP_FE(git_status_list_entrycount, arginfo_git_status_list_entrycount)
	PHP_FE(git_status_byindex, arginfo_git_status_byindex)
	PHP_FE(git_status_list_free, arginfo_git_status_list_free)
	PHP_FE(git_status_should_ignore, arginfo_git_status_should_ignore)
	PHP_FE(git_status_options_new, NULL)

	/* transport */
	PHP_FE(git_transport_new, arginfo_git_transport_new)
	PHP_FE(git_transport_register, arginfo_git_transport_register)
	PHP_FE(git_transport_unregister, arginfo_git_transport_unregister)
	PHP_FE(git_transport_dummy, arginfo_git_transport_dummy)
	PHP_FE(git_transport_local, arginfo_git_transport_local)
	PHP_FE(git_transport_smart, arginfo_git_transport_smart)
	PHP_FE(git_smart_subtransport_http, arginfo_git_smart_subtransport_http)
	PHP_FE(git_smart_subtransport_git, arginfo_git_smart_subtransport_git)
	PHP_FE(git_smart_subtransport_ssh, arginfo_git_smart_subtransport_ssh)

	/* diff */
	PHP_FE(git_diff_free, arginfo_git_diff_free)
	PHP_FE(git_diff_tree_to_tree, arginfo_git_diff_tree_to_tree)
	PHP_FE(git_diff_tree_to_index, arginfo_git_diff_tree_to_index)
	PHP_FE(git_diff_index_to_workdir, arginfo_git_diff_index_to_workdir)
	PHP_FE(git_diff_tree_to_workdir, arginfo_git_diff_tree_to_workdir)
	PHP_FE(git_diff_tree_to_workdir_with_index, arginfo_git_diff_tree_to_workdir_with_index)
	PHP_FE(git_diff_merge, arginfo_git_diff_merge)
	PHP_FE(git_diff_find_similar, arginfo_git_diff_find_similar)
	PHP_FE(git_diff_options_init, arginfo_git_diff_options_init)
	PHP_FE(git_diff_num_deltas, arginfo_git_diff_num_deltas)
	PHP_FE(git_diff_num_deltas_of_type, arginfo_git_diff_num_deltas_of_type)
	PHP_FE(git_diff_get_delta, arginfo_git_diff_get_delta)
	PHP_FE(git_diff_is_sorted_icase, arginfo_git_diff_is_sorted_icase)
	PHP_FE(git_diff_foreach, arginfo_git_diff_foreach)
	PHP_FE(git_diff_status_char, arginfo_git_diff_status_char)
	PHP_FE(git_diff_print, arginfo_git_diff_print)
	PHP_FE(git_diff_blobs, arginfo_git_diff_blobs)
	PHP_FE(git_diff_blob_to_buffer, arginfo_git_diff_blob_to_buffer)

	/* checkout */
	PHP_FE(git_checkout_head, arginfo_git_checkout_head)
	PHP_FE(git_checkout_index, arginfo_git_checkout_index)
	PHP_FE(git_checkout_tree, arginfo_git_checkout_tree)

	PHP_FE(git_checkout_opts_new, NULL) /* convention function */

	/* filter */
	PHP_FE(git_filter_list_load, arginfo_git_filter_list_load)
	PHP_FE(git_filter_list_apply_to_data, arginfo_git_filter_list_apply_to_data)
	PHP_FE(git_filter_list_apply_to_file, arginfo_git_filter_list_apply_to_file)
	PHP_FE(git_filter_list_apply_to_blob, arginfo_git_filter_list_apply_to_blob)
	PHP_FE(git_filter_list_free, arginfo_git_filter_list_free)
	PHP_FE(git_filter_lookup, arginfo_git_filter_lookup)
	PHP_FE(git_filter_list_new, arginfo_git_filter_list_new)
	PHP_FE(git_filter_list_push, arginfo_git_filter_list_push)
	PHP_FE(git_filter_list_length, arginfo_git_filter_list_length)
	PHP_FE(git_filter_source_repo, arginfo_git_filter_source_repo)
	PHP_FE(git_filter_source_path, arginfo_git_filter_source_path)
	PHP_FE(git_filter_source_filemode, arginfo_git_filter_source_filemode)
	PHP_FE(git_filter_source_id, arginfo_git_filter_source_id)
	PHP_FE(git_filter_source_mode, arginfo_git_filter_source_mode)
	PHP_FE(git_filter_register, arginfo_git_filter_register)
	PHP_FE(git_filter_unregister, arginfo_git_filter_unregister)
	PHP_FE(git_filter_new, arginfo_git_filter_new)

	/* ignore */
	PHP_FE(git_ignore_add_rule, arginfo_git_ignore_add_rule)
	PHP_FE(git_ignore_clear_internal_rules, arginfo_git_ignore_clear_internal_rules)
	PHP_FE(git_ignore_path_is_ignored, arginfo_git_ignore_path_is_ignored)

	/* indexer */
	PHP_FE(git_indexer_new, arginfo_git_indexer_new)
	PHP_FE(git_indexer_append, arginfo_git_indexer_append)
	PHP_FE(git_indexer_commit, arginfo_git_indexer_commit)
	PHP_FE(git_indexer_hash, arginfo_git_indexer_hash)
	PHP_FE(git_indexer_free, arginfo_git_indexer_free)

	/* pathspec */
	PHP_FE(git_pathspec_new, arginfo_git_pathspec_new)
	PHP_FE(git_pathspec_free, arginfo_git_pathspec_free)
	PHP_FE(git_pathspec_matches_path, arginfo_git_pathspec_matches_path)
	PHP_FE(git_pathspec_match_workdir, arginfo_git_pathspec_match_workdir)
	PHP_FE(git_pathspec_match_index, arginfo_git_pathspec_match_index)
	PHP_FE(git_pathspec_match_tree, arginfo_git_pathspec_match_tree)
	PHP_FE(git_pathspec_match_diff, arginfo_git_pathspec_match_diff)
	PHP_FE(git_pathspec_match_list_free, arginfo_git_pathspec_match_list_free)
	PHP_FE(git_pathspec_match_list_entrycount, arginfo_git_pathspec_match_list_entrycount)
	PHP_FE(git_pathspec_match_list_entry, arginfo_git_pathspec_match_list_entry)
	PHP_FE(git_pathspec_match_list_diff_entry, arginfo_git_pathspec_match_list_diff_entry)
	PHP_FE(git_pathspec_match_list_failed_entrycount, arginfo_git_pathspec_match_list_failed_entrycount)
	PHP_FE(git_pathspec_match_list_failed_entry, arginfo_git_pathspec_match_list_failed_entry)

	/* patch */
	PHP_FE(git_patch_from_diff, arginfo_git_patch_from_diff)
	PHP_FE(git_patch_from_blobs, arginfo_git_patch_from_blobs)
	PHP_FE(git_patch_from_blob_and_buffer, arginfo_git_patch_from_blob_and_buffer)
	PHP_FE(git_patch_free, arginfo_git_patch_free)
	PHP_FE(git_patch_get_delta, arginfo_git_patch_get_delta)
	PHP_FE(git_patch_num_hunks, arginfo_git_patch_num_hunks)
	PHP_FE(git_patch_line_stats, arginfo_git_patch_line_stats)
	PHP_FE(git_patch_get_hunk, arginfo_git_patch_get_hunk)
	PHP_FE(git_patch_num_lines_in_hunk, arginfo_git_patch_num_lines_in_hunk)
	PHP_FE(git_patch_get_line_in_hunk, arginfo_git_patch_get_line_in_hunk)
	PHP_FE(git_patch_size, arginfo_git_patch_size)
	PHP_FE(git_patch_print, arginfo_git_patch_print)
	PHP_FE(git_patch_to_str, arginfo_git_patch_to_str)

	/* annotated_commit */
	PHP_FE(git_annotated_commit_from_ref, arginfo_git_annotated_commit_from_ref)
	PHP_FE(git_annotated_commit_from_fetchhead, arginfo_git_annotated_commit_from_fetchhead)
	PHP_FE(git_annotated_commit_free, arginfo_git_annotated_commit_free)
	PHP_FE(git_annotated_commit_lookup, arginfogit_annotated_commit_lookup)

	/* merge */
	PHP_FE(git_merge_base, arginfo_git_merge_base)
	PHP_FE(git_merge_base_many, arginfo_git_merge_base_many)
	PHP_FE(git_merge_trees, arginfo_git_merge_trees)
	PHP_FE(git_merge, arginfo_git_merge)
	PHP_FE(git_merge_result_is_uptodate, arginfo_git_merge_result_is_uptodate)
	PHP_FE(git_merge_result_is_fastforward, arginfo_git_merge_result_is_fastforward)
	PHP_FE(git_merge_result_fastforward_oid, arginfo_git_merge_result_fastforward_oid)
	PHP_FE(git_merge_result_free, arginfo_git_merge_result_free)

	/* tag */
	PHP_FE(git_tag_lookup, arginfo_git_tag_lookup)
	PHP_FE(git_tag_lookup_prefix, arginfo_git_tag_lookup_prefix)
	PHP_FE(git_tag_free, arginfo_git_tag_free)
	PHP_FE(git_tag_id, arginfo_git_tag_id)
	PHP_FE(git_tag_owner, arginfo_git_tag_owner)
	PHP_FE(git_tag_target, arginfo_git_tag_target)
	PHP_FE(git_tag_target_id, arginfo_git_tag_target_id)
	PHP_FE(git_tag_target_type, arginfo_git_tag_target_type)
	PHP_FE(git_tag_name, arginfo_git_tag_name)
	PHP_FE(git_tag_tagger, arginfo_git_tag_tagger)
	PHP_FE(git_tag_message, arginfo_git_tag_message)
	PHP_FE(git_tag_create, arginfo_git_tag_create)
	PHP_FE(git_tag_annotation_create, arginfo_git_tag_annotation_create)
	PHP_FE(git_tag_create_frombuffer, arginfo_git_tag_create_frombuffer)
	PHP_FE(git_tag_create_lightweight, arginfo_git_tag_create_lightweight)
	PHP_FE(git_tag_delete, arginfo_git_tag_delete)
	PHP_FE(git_tag_list, arginfo_git_tag_list)
	PHP_FE(git_tag_list_match, arginfo_git_tag_list_match)
	PHP_FE(git_tag_foreach, arginfo_git_tag_foreach)
	PHP_FE(git_tag_peel, arginfo_git_tag_peel)

	/* note */
	PHP_FE(git_note_iterator_new, arginfo_git_note_iterator_new)
	PHP_FE(git_note_iterator_free, arginfo_git_note_iterator_free)
	PHP_FE(git_note_next, arginfo_git_note_next)
	PHP_FE(git_note_read, arginfo_git_note_read)
	PHP_FE(git_note_message, arginfo_git_note_message)
	PHP_FE(git_note_oid, arginfo_git_note_oid)
	PHP_FE(git_note_create, arginfo_git_note_create)
	PHP_FE(git_note_remove, arginfo_git_note_remove)
	PHP_FE(git_note_free, arginfo_git_note_free)
	PHP_FE(git_note_default_ref, arginfo_git_note_default_ref)
	PHP_FE(git_note_foreach, arginfo_git_note_foreach)

	/* odb */
	PHP_FE(git_odb_new, arginfo_git_odb_new)
	PHP_FE(git_odb_open, arginfo_git_odb_open)
	PHP_FE(git_odb_add_disk_alternate, arginfo_git_odb_add_disk_alternate)
	PHP_FE(git_odb_free, arginfo_git_odb_free)
	PHP_FE(git_odb_read, arginfo_git_odb_read)
	PHP_FE(git_odb_read_prefix, arginfo_git_odb_read_prefix)
	PHP_FE(git_odb_read_header, arginfo_git_odb_read_header)
	PHP_FE(git_odb_exists, arginfo_git_odb_exists)
	PHP_FE(git_odb_refresh, arginfo_git_odb_refresh)
	PHP_FE(git_odb_foreach, arginfo_git_odb_foreach)
	PHP_FE(git_odb_write, arginfo_git_odb_write)
	PHP_FE(git_odb_open_wstream, arginfo_git_odb_open_wstream)
	PHP_FE(git_odb_stream_write, arginfo_git_odb_stream_write)
	PHP_FE(git_odb_stream_finalize_write, arginfo_git_odb_stream_finalize_write)
	PHP_FE(git_odb_stream_read, arginfo_git_odb_stream_read)
	PHP_FE(git_odb_stream_free, arginfo_git_odb_stream_free)
	PHP_FE(git_odb_open_rstream, arginfo_git_odb_open_rstream)
	PHP_FE(git_odb_write_pack, arginfo_git_odb_write_pack)
	PHP_FE(git_odb_hash, arginfo_git_odb_hash)
	PHP_FE(git_odb_hashfile, arginfo_git_odb_hashfile)
	PHP_FE(git_odb_object_dup, arginfo_git_odb_object_dup)
	PHP_FE(git_odb_object_free, arginfo_git_odb_object_free)
	PHP_FE(git_odb_object_id, arginfo_git_odb_object_id)
	PHP_FE(git_odb_object_data, arginfo_git_odb_object_data)
	PHP_FE(git_odb_object_size, arginfo_git_odb_object_size)
	PHP_FE(git_odb_object_type, arginfo_git_odb_object_type)
	PHP_FE(git_odb_add_backend, arginfo_git_odb_add_backend)
	PHP_FE(git_odb_add_alternate, arginfo_git_odb_add_alternate)
	PHP_FE(git_odb_num_backends, arginfo_git_odb_num_backends)
	PHP_FE(git_odb_get_backend, arginfo_git_odb_get_backend)

	PHP_FE(git_odb_backend_new, arginfo_git_odb_backend_new)

	/* reflog */
	PHP_FE(git_reflog_read, arginfo_git_reflog_read)
	PHP_FE(git_reflog_write, arginfo_git_reflog_write)
	PHP_FE(git_reflog_append, arginfo_git_reflog_append)
	PHP_FE(git_reflog_append_to, arginfo_git_reflog_append_to)
	PHP_FE(git_reflog_rename, arginfo_git_reflog_rename)
	PHP_FE(git_reflog_delete, arginfo_git_reflog_delete)
	PHP_FE(git_reflog_entrycount, arginfo_git_reflog_entrycount)
	PHP_FE(git_reflog_entry_byindex, arginfo_git_reflog_entry_byindex)
	PHP_FE(git_reflog_drop, arginfo_git_reflog_drop)
	PHP_FE(git_reflog_entry_id_old, arginfo_git_reflog_entry_id_old)
	PHP_FE(git_reflog_entry_id_new, arginfo_git_reflog_entry_id_new)
	PHP_FE(git_reflog_entry_committer, arginfo_git_reflog_entry_committer)
	PHP_FE(git_reflog_entry_message, arginfo_git_reflog_entry_message)
	PHP_FE(git_reflog_free, arginfo_git_reflog_free)

	/* packbuilder */
	PHP_FE(git_packbuilder_new, arginfo_git_packbuilder_new)
	PHP_FE(git_packbuilder_set_threads, arginfo_git_packbuilder_set_threads)
	PHP_FE(git_packbuilder_insert, arginfo_git_packbuilder_insert)
	PHP_FE(git_packbuilder_insert_tree, arginfo_git_packbuilder_insert_tree)
	PHP_FE(git_packbuilder_insert_commit, arginfo_git_packbuilder_insert_commit)
	PHP_FE(git_packbuilder_write, arginfo_git_packbuilder_write)
	PHP_FE(git_packbuilder_hash, arginfo_git_packbuilder_hash)
	PHP_FE(git_packbuilder_foreach, arginfo_git_packbuilder_foreach)
	PHP_FE(git_packbuilder_object_count, arginfo_git_packbuilder_object_count)
	PHP_FE(git_packbuilder_written, arginfo_git_packbuilder_written)
	PHP_FE(git_packbuilder_set_callbacks, arginfo_git_packbuilder_set_callbacks)
	PHP_FE(git_packbuilder_free, arginfo_git_packbuilder_free)

	/* stash */
	PHP_FE(git_stash_save, arginfo_git_stash_save)
	PHP_FE(git_stash_foreach, arginfo_git_stash_foreach)
	PHP_FE(git_stash_drop, arginfo_git_stash_drop)

	/* signature */
	PHP_FE(git_signature_new, arginfo_git_signature_new)
	PHP_FE(git_signature_now, arginfo_git_signature_now)
	PHP_FE(git_signature_default, arginfo_git_signature_default)

	/* reset */
	PHP_FE(git_reset, arginfo_git_reset)
	PHP_FE(git_reset_default, arginfo_git_reset_default)

	/* message */
	PHP_FE(git_message_prettify, arginfo_git_message_prettify)

	/* submodule */
	PHP_FE(git_submodule_lookup, arginfo_git_submodule_lookup)
	PHP_FE(git_submodule_foreach, arginfo_git_submodule_foreach)
	PHP_FE(git_submodule_add_setup, arginfo_git_submodule_add_setup)
	PHP_FE(git_submodule_add_finalize, arginfo_git_submodule_add_finalize)
	PHP_FE(git_submodule_add_to_index, arginfo_git_submodule_add_to_index)
	PHP_FE(git_submodule_save, arginfo_git_submodule_save)
	PHP_FE(git_submodule_owner, arginfo_git_submodule_owner)
	PHP_FE(git_submodule_name, arginfo_git_submodule_name)
	PHP_FE(git_submodule_path, arginfo_git_submodule_path)
	PHP_FE(git_submodule_url, arginfo_git_submodule_url)
	PHP_FE(git_submodule_set_url, arginfo_git_submodule_set_url)
	PHP_FE(git_submodule_index_id, arginfo_git_submodule_index_id)
	PHP_FE(git_submodule_head_id, arginfo_git_submodule_head_id)
	PHP_FE(git_submodule_wd_id, arginfo_git_submodule_wd_id)
	PHP_FE(git_submodule_ignore, arginfo_git_submodule_ignore)
	PHP_FE(git_submodule_set_ignore, arginfo_git_submodule_set_ignore)
	PHP_FE(git_submodule_update, arginfo_git_submodule_update)
	PHP_FE(git_submodule_set_update, arginfo_git_submodule_set_update)
	PHP_FE(git_submodule_fetch_recurse_submodules, arginfo_git_submodule_fetch_recurse_submodules)
	PHP_FE(git_submodule_set_fetch_recurse_submodules, arginfo_git_submodule_set_fetch_recurse_submodules)
	PHP_FE(git_submodule_init, arginfo_git_submodule_init)
	PHP_FE(git_submodule_sync, arginfo_git_submodule_sync)
	PHP_FE(git_submodule_open, arginfo_git_submodule_open)
	PHP_FE(git_submodule_reload, arginfo_git_submodule_reload)
	PHP_FE(git_submodule_reload_all, arginfo_git_submodule_reload_all)
	PHP_FE(git_submodule_status, arginfo_git_submodule_status)
	PHP_FE(git_submodule_location, arginfo_git_submodule_location)

	/* attr */
	PHP_FE(git_attr_value, arginfo_git_attr_value)
	PHP_FE(git_attr_get, arginfo_git_attr_get)
	PHP_FE(git_attr_get_many, arginfo_git_attr_get_many)
	PHP_FE(git_attr_foreach, arginfo_git_attr_foreach)
	PHP_FE(git_attr_cache_flush, arginfo_git_attr_cache_flush)
	PHP_FE(git_attr_add_macro, arginfo_git_attr_add_macro)

	/* giterr */
	PHP_FE(giterr_last, arginfo_giterr_last)
	PHP_FE(giterr_clear, arginfo_giterr_clear)
	PHP_FE(giterr_detach, arginfo_giterr_detach)
	PHP_FE(giterr_set_str, arginfo_giterr_set_str)
	PHP_FE(giterr_set_oom, arginfo_giterr_set_oom)

	/* push */
	PHP_FE(git_push_new, arginfo_git_push_new)
	PHP_FE(git_push_set_options, arginfo_git_push_set_options)
	PHP_FE(git_push_set_callbacks, arginfo_git_push_set_callbacks)
	PHP_FE(git_push_add_refspec, arginfo_git_push_add_refspec)
	PHP_FE(git_push_update_tips, arginfo_git_push_update_tips)
	PHP_FE(git_push_finish, arginfo_git_push_finish)
	PHP_FE(git_push_unpack_ok, arginfo_git_push_unpack_ok)
	PHP_FE(git_push_status_foreach, arginfo_git_push_status_foreach)
	PHP_FE(git_push_free, arginfo_git_push_free)

	/* refspec */
	PHP_FE(git_refspec_src, arginfo_git_refspec_src)
	PHP_FE(git_refspec_dst, arginfo_git_refspec_dst)
	PHP_FE(git_refspec_string, arginfo_git_refspec_string)
	PHP_FE(git_refspec_force, arginfo_git_refspec_force)
	PHP_FE(git_refspec_direction, arginfo_git_refspec_direction)
	PHP_FE(git_refspec_src_matches, arginfo_git_refspec_src_matches)
	PHP_FE(git_refspec_dst_matches, arginfo_git_refspec_dst_matches)
	PHP_FE(git_refspec_transform, arginfo_git_refspec_transform)
	PHP_FE(git_refspec_rtransform, arginfo_git_refspec_rtransform)

	/* graph */
	PHP_FE(git_graph_ahead_behind, arginfo_git_graph_ahead_behind)

	/* blame */
	PHP_FE(git_blame_get_hunk_count, arginfo_git_blame_get_hunk_count)
	PHP_FE(git_blame_get_hunk_byindex, arginfo_git_blame_get_hunk_byindex)
	PHP_FE(git_blame_get_hunk_byline, arginfo_git_blame_get_hunk_byline)
	PHP_FE(git_blame_file, arginfo_git_blame_file)
	PHP_FE(git_blame_buffer, arginfo_git_blame_buffer)
	PHP_FE(git_blame_free, arginfo_git_blame_free)
	PHP_FE(git_blame_options_new, arginfo_git_blame_options_new)

	/* misc */
	PHP_FE(git_resource_type, arginfo_git_resource_type)
	PHP_FE(git_libgit2_capabilities, NULL)
	PHP_FE(git_libgit2_version, NULL)
	PHP_FE_END
};

PHP_MINFO_FUNCTION(git2)
{
	char buf[32] = {0};
	int major, minor, rev;

	php_printf("PHP Git2 Extension\n");

	git_libgit2_version(&major, &minor, &rev);
	snprintf(buf, 32, "%d.%d.%d", major, minor, rev);

	php_info_print_table_start();
	php_info_print_table_header(2, "Git2 Support", "enabled");
	php_info_print_table_header(2, "libgit2 version", buf);
	php_info_print_table_end();
}

PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("git2.dummy", "1", PHP_INI_ALL, OnUpdateLong, dummy, zend_git2_globals, git2_globals)
PHP_INI_END()

static PHP_GINIT_FUNCTION(git2)
{
}

static PHP_GSHUTDOWN_FUNCTION(git2)
{
}


static void php_git2_odb_backend_foreach_callback_free_storage(php_git2_odb_backend_foreach_callback *object TSRMLS_DC)
{
        zend_object_std_dtor(&object->zo TSRMLS_CC);
        efree(object);
}

zend_object_value php_git2_odb_backend_foreach_callback_new(zend_class_entry *ce TSRMLS_DC)
{
        zend_object_value retval;
        PHP_GIT2_STD_CREATE_OBJECT(php_git2_odb_backend_foreach_callback);
        return retval;
}

PHP_MINIT_FUNCTION(git2)
{
	zend_class_entry ce;
	REGISTER_INI_ENTRIES();


	INIT_CLASS_ENTRY(ce, "Git2ODBBackendForeachCallback", 0);
	php_git2_odb_backend_foreach_callback_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	zend_register_class_alias_ex(ZEND_NS_NAME("Git2\\ODB\\Backend", "ForeachCallback"), sizeof(ZEND_NS_NAME("Git2\\ODB\\Backend", "ForeachCallback"))-1, php_git2_odb_backend_foreach_callback_class_entry TSRMLS_CC);
	php_git2_odb_backend_foreach_callback_class_entry->create_object = php_git2_odb_backend_foreach_callback_new;

	git2_resource_handle = zend_register_list_destructors_ex(destruct_git2, NULL, PHP_GIT2_RESOURCE_NAME, module_number);

	REGISTER_LONG_CONSTANT("GIT_TYPE_REPOSITORY", PHP_GIT2_TYPE_REPOSITORY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_COMMIT", PHP_GIT2_TYPE_COMMIT,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_TREE", PHP_GIT2_TYPE_TREE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_TREE_ENTRY", PHP_GIT2_TYPE_TREE_ENTRY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_BLOB", PHP_GIT2_TYPE_BLOB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_REVWALK", PHP_GIT2_TYPE_REVWALK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_TREEBUILDER", PHP_GIT2_TYPE_TREEBUILDER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_REFERENCE", PHP_GIT2_TYPE_REFERENCE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_CONFIG", PHP_GIT2_TYPE_CONFIG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_OBJECT", PHP_GIT2_TYPE_OBJECT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_INDEX", PHP_GIT2_TYPE_INDEX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_ODB", PHP_GIT2_TYPE_ODB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_REFDB", PHP_GIT2_TYPE_REFDB, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_BRANCH_ITERATOR", PHP_GIT2_TYPE_BRANCH_ITERATOR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_TAG", PHP_GIT2_TYPE_TAG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_CRED", PHP_GIT2_TYPE_CRED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_TRANSPORT", PHP_GIT2_TYPE_TRANSPORT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_REMOTE", PHP_GIT2_TYPE_REMOTE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_DIFF", PHP_GIT2_TYPE_DIFF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_MERGE_RESULT", PHP_GIT2_TYPE_MERGE_RESULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_ANNOTATED_COMMIT", PHP_GIT2_TYPE_ANNOTATED_COMMIT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_PATHSPEC", PHP_GIT2_TYPE_PATHSPEC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_PATHSPEC_MATCH_LIST", PHP_GIT2_TYPE_PATHSPEC_MATCH_LIST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_PATCH", PHP_GIT2_TYPE_PATCH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_DIFF_HUNK", PHP_GIT2_TYPE_DIFF_HUNK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_STATUS_LIST", PHP_GIT2_TYPE_STATUS_LIST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_BUF", PHP_GIT2_TYPE_BUF, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_FILTER_LIST", PHP_GIT2_TYPE_FILTER_LIST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_FILTER_SOURCE", PHP_GIT2_TYPE_FILTER_SOURCE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("GIT_TYPE_DIFF_LINE", PHP_GIT2_TYPE_DIFF_LINE, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}

PHP_RINIT_FUNCTION(git2)
{
	git_libgit2_init();
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(git2)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(git2)
{
	git_libgit2_shutdown();
	return SUCCESS;
}

zend_module_entry git2_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_GIT2_EXTNAME,
	php_git2_functions,					/* Functions */
	PHP_MINIT(git2),	/* MINIT */
	PHP_MSHUTDOWN(git2),	/* MSHUTDOWN */
	PHP_RINIT(git2),	/* RINIT */
	PHP_RSHUTDOWN(git2),		/* RSHUTDOWN */
	PHP_MINFO(git2),	/* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
	PHP_GIT2_EXTVER,
#endif
	PHP_MODULE_GLOBALS(git2),
	PHP_GINIT(git2),
	PHP_GSHUTDOWN(git2),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_GIT2
ZEND_GET_MODULE(git2)
#endif