<?php

/**
 * @File
 * identifier的分派
 */

/** 
 *	新增資料集時指定詮釋資料資料集序號
 *  @param 機關別分類(org)的tid
 */
function ndcop_dataset_api_assign_dataset_identifier($tid){

	 $term_obj = taxonomy_term_load($tid);
	 $org_oid = $term_obj->field_orgoid[LANGUAGE_NONE][0]['value']; 
	 $oid_length = strlen($org_oid);

	 	// 1|機關, 2|處室單位
	 	// 機關
	 	if($term_obj->field_is_org[LANGUAGE_NONE][0]['value'] == '1'){
	 		// 本身機關代碼
	 		$org_num = $term_obj->field_org_num[LANGUAGE_NONE][0]['value'];
	 		$serial_num = $term_obj->field_serial_num[LANGUAGE_NONE][0]['value'];

	 		// 更新本身的機關別分類序號
			$term_obj->field_serial_num[LANGUAGE_NONE][0]['value'] = $term_obj->field_serial_num[LANGUAGE_NONE][0]['value'] +1 ;
			taxonomy_term_save($term_obj);
	 	}
	 	// 處室單位
	 	elseif($term_obj->field_is_org[LANGUAGE_NONE][0]['value'] == '2'){
	 		try{
		 		// 處室單位取得所屬管理機關
		 		$parent_org_tid = ndcop_dataset_api_get_parent_org($org_oid);
		 		$parent_org_term = taxonomy_term_load($parent_org_tid);
		 		$org_num = $parent_org_term->field_org_num[LANGUAGE_NONE][0]['value'];
	 			$serial_num = $parent_org_term->field_serial_num[LANGUAGE_NONE][0]['value'];

	 			// 更新所屬管理機關的機關別分類序號
				$parent_org_term->field_serial_num[LANGUAGE_NONE][0]['value'] = $parent_org_term->field_serial_num[LANGUAGE_NONE][0]['value'] + 1 ;
				taxonomy_term_save($parent_org_term);
	 			
			}catch(Exception $e){
				echo ($e->getMessage());
				drupal_set_message($e->getMessage(), 'error');
        		return null;
			}
	 	}
	 	else{
	 		throw new Exception("尚未設定機關別分類的是否為機關欄位");
	 	}

	 // 產生資料集序號
	 $get_this_serial_no = ndcop_dataset_api_generate_dataset_identifier($org_num, $serial_num, 6);

	 return $get_this_serial_no;
	
}

/** 
 * 處室單位取得所屬管理機關
 * ex. 法務部統計處取得法務部
 */
function ndcop_dataset_api_get_parent_org($org_oid){
	$oid_split = array();
	$oid_split = explode('.',$org_oid);
	// popup last element
	array_pop($oid_split);
	// 組合成上層機關OID
	$parent_oid = implode('.', $oid_split);

	// DB Access: Get taxonomy_term By org
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'taxonomy_term')
  	  ->entityCondition('bundle', array('org'))
  	  ->fieldCondition('field_orgoid', 'value', $parent_oid, '=');
	$result = $query->execute();
	$parent_tid = array_pop( array_keys($result['taxonomy_term']) );

	if( !empty($parent_tid) )
		return $parent_tid;
	else
		throw new Exception("於新增資料集處理資料集序號時發現parent_oid 尚未新增於本平臺機關別分類中");
}

/** 
 * 產生資料集序號
 * @param $org_num: 機關代碼
 *		  $serial_num: 序號
 *        $complete_length: 完整長度         
 * @return 資料集序號
 */
function ndcop_dataset_api_generate_dataset_identifier($org_num, $serial_num, $complete_length){

	$serial_num_length = strlen($serial_num);
	 //echo strlen($serial_num);

	if(is_numeric($complete_length) and $complete_length > 0 ){
		if( $complete_length > $serial_num_length ){

		 	// 補0長度
		 	$zero_length = $complete_length - $serial_num_length;

		 	for ($i=0 ; $i < $zero_length ; $i++) {
		 		$serial_num = '0'. $serial_num;
		 	}

		}
		elseif( $complete_length == $serial_num_length ){

		 	$error_msg = '資料集編號 almost too long ,its time for programmer to re-write our module.';
			watchdog('ndcop_dataset_api', $error_msg, array(), WATCHDOG_ALERT);
			throw new Exception($error_msg);
		}
		else{
		 	$error_msg = '資料集編號 almost too long ,its time for programmer to re-write our module.';
			watchdog('ndcop_dataset_api', $error_msg, array(), WATCHDOG_ALERT);
			throw new Exception($error_msg);
		}

	}
	else{
		$error_msg = 'ERROR ndcop_dataset_api_generate_dataset_identifier $complete_length.';
		watchdog('ndcop_dataset_api', $error_msg, array(), WATCHDOG_ALERT);
		throw new Exception($error_msg);
	}
	 return $org_num.'-'.$serial_num;
}

/** 
 * 產生資料項目(resource)序號
 * @param $dataset_num: 資料集編號
 *		  $serial_num: 序號
 *        $complete_length: 完整長度         
 * @return 資料項目(resource)序號
 */
function ndcop_dataset_api_generate_resource_id($dataset_identifier, $serial_num, $complete_length){

	$serial_num_length = strlen($serial_num);
	 //echo strlen($serial_num);

	if(is_numeric($complete_length) and $complete_length > 0 ){
		if( $complete_length > $serial_num_length ){

		 	$zero_length = $complete_length - $serial_num_length;

		 	for ($i=0 ; $i < $zero_length ; $i++) {
		 		$serial_num = '0'. $serial_num;
		 	}

		}
		elseif( $complete_length == $serial_num_length ){

		 	$error_msg = '資料項目編號 almost too long ,its time for programmer to re-write our module.';
			watchdog('ndcop_dataset_api', $error_msg, array(), WATCHDOG_ALERT);
			throw new Exception($error_msg);
		}
		else{
		 	$error_msg = '資料項目編號 almost too long ,its time for programmer to re-write our module.';
			watchdog('ndcop_dataset_api', $error_msg, array(), WATCHDOG_ALERT);
			throw new Exception($error_msg);
		}

	}
	else{
		$error_msg = 'ERROR ndcop_dataset_api_generate_resource_id $complete_length.';
		watchdog('ndcop_dataset_api', $error_msg, array(), WATCHDOG_ALERT);
		throw new Exception($error_msg);
	}
	 return $dataset_identifier.'-'.$serial_num;
}