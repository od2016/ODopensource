<?php

/**
 * @file
 * 負責處理意見回覆(comment).
 * TODO:
 */

/**
*  新增資料集時新增Comments
*  @see ndcop_dataset_api_create_dataset
*  @param $dataset: Host Entity
*  @param $Comments: NDCOPDATA規範的 API形式的Comments Array
*/
function ndcop_dataset_api_create_comment($dataset,$comments,$update=false){
	$nid = $dataset->nid;
	$ccid_list = array();

	foreach($comments as $key => $comment) {
		$commentObject = new stdClass();
		$ccid = $comment->commentId;
		array_push($ccid_list, $ccid);
		$cid = ndcop_api_get_commentId($nid,$ccid);
		$commentObject->nid = $nid;
		$commentObject->mail = $comment->commentEmail;
		$commentObject->name = $comment->commentAuthor;
		$commentObject->hostname = $comment->commentHost;
		$commentObject->is_anonymous = 1;
		$commentObject->subject = $comment->commentSubject;
		$commentObject->language = LANGUAGE_NONE;
		$commentObject->comment_body[LANGUAGE_NONE][0]['value'] = $comment->comments;
		$commentObject->comment_body[LANGUAGE_NONE][0]['format'] = 'filtered_html';
		$commentObject->{NDCOP_API_COMMENT_TYPE}[LANGUAGE_NONE][0]['value'] = '7';
		$commentObject->{NDCOP_API_COMMENT_TIME}[LANGUAGE_NONE][0]['value'] = strtotime($comment->commentTime);
		$commentObject->{NDCOP_API_COMMENT_HOST}[LANGUAGE_NONE][0]['value'] = $comment->commentHost;
		$commentObject->{NDCOP_API_COMMENT_CID}[LANGUAGE_NONE][0]['value'] = $ccid;
		$cpid = $comment->commentReplyId;
		$pid=ndcop_dataset_api_set_pid($nid,$cpid);
		$commentObject->pid = $pid;
		$commentObject->{NDCOP_API_COMMENT_PID}[LANGUAGE_NONE][0]['value'] = $cpid;
		if(!is_null($cid)){
			$commentObject->cid = $cid;
		}
		$commentObject->{field_dataset_modify_method}[LANGUAGE_NONE][0]['value'] = '1';
		$commetn_sub = comment_submit($commentObject); // saving a comment
		comment_save($commetn_sub);
		unset($commentObject);
	}
	if($update){
		$items = comment_get_thread($dataset,'COMMENT_MODE_FLAT','99');
		$comments_array=(comment_load_multiple($items));
		$delete_list = array();
		foreach ($comments_array as $item) {
			$ccid='';
			$cid='';
			$ccid=$item->{NDCOP_API_COMMENT_CID}[LANGUAGE_NONE][0]['safe_value'];
			if(!in_array($ccid,$ccid_list)){
				$cid = ndcop_api_get_commentId($nid,$ccid);
				array_push($delete_list, $cid);
			}
		}
		comment_delete_multiple($delete_list);
	}
}

function ndcop_api_get_commentId($nid,$ccid){
	$query = db_select('field_data_field_cid_api', 'ccid');
	$query->fields('ccid',array('entity_id'));
	$query->join('comment', 'com', 'ccid.entity_id = com.cid');
	$query->condition('ccid.field_cid_api_value',$ccid,'=');
	$query->condition('com.nid',$nid,'=');
	$result = $query->execute()->fetchAssoc();
	$cid=$result['entity_id'];
	return $cid;
}

function ndcop_dataset_api_set_pid($nid,$cpid){
	$query = db_select('field_data_field_cid_api', 'ccid');
	$query->fields('ccid',array('entity_id'));
	$query->join('comment', 'com', 'ccid.entity_id = com.cid');
	$query->condition('ccid.field_cid_api_value',$cpid,'=');
	$query->condition('com.nid',$nid,'=');
	$result = $query->execute()->fetchAssoc();
	$pid=$result['entity_id'];
	if(is_null($pid))
		return '0';
	return $pid;
}