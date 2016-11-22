<?php

/**
 * @file
 * 負責處理API驗證機制.
 * TODO:
 */

/**
 * 是否為正整數.
 */
function ndcop_api_validate_postive_int($num){

	if(is_numeric($num) && intval($num) > 0){
		return TRUE;
	}else{
		return FALSE;
	}
}

/**
 * 是否為catagoryCode.
 * @return Boolean
 */
function ndcop_api_validate_is_catagoryCode($str){

	// Get System All catagoryCode
	$tids = ndcop_dataset_api_get_category_tids(0,'');
	$category_codes = array();
	foreach ($tids as $key => $tid) {
		$category_term_obj = taxonomy_term_load($tid);
		$category_codes[] = $category_term_obj->field_code_id[LANGUAGE_NONE][0]['safe_value'];
	}
	return in_array($str, $category_codes);
}

/**
 * 是否在平台的標籤列表中.
 * @return if exist ,tid
 *         if not exist, 0
 */
function ndcop_api_validate_in_tag_list($tag){

	// 取得Array key:term name, value:tid
	$taxonomy_keyword_terms = ndcop_api_get_taxonomy_term_names('tags');
	$tag_tid = ( !empty($taxonomy_keyword_terms[$tag]) ) ? $taxonomy_keyword_terms[$tag] : 0 ;
		return $tag_tid;
	}
/**
 * 驗證標籤是否存在dataset_tags
 */
function ndcop_api_validate_tag_in_dataset_tags($tag){
    $rst = db_query("SELECT * FROM {field_data_field_dataset_tags}
                     WHERE field_dataset_tags_tag = :tag", array(':tag' => $tag))
         ->fetchAssoc();
  return $rst;
}
/**
 * 是否在平台的serial no列表中.
 * @return if exist ,nid
 *         if not exist, 0
 */
function ndcop_api_validate_in_identifier_list($id){

	$identifiers = array();
	$count = 0;
	// Get System All dataset identifier
	$results = ndcop_dataset_api_get_dataset('',0,'');

	foreach ($results as $result) {
		$identifiers[$result->serialno] = $result->nid;
		$count++;
	}
	$dataset_nid = ( !empty($identifiers[$id]) ) ? $identifiers[$id] : 0 ;
	return $dataset_nid;
}

/**
 * 檢查是否有該commentiId
 * @return if exist ,cid
 *         if not exist, 0
 */
function ndcop_api_validate_in_comments_list($id){

	$identifiers = array();
	$count = 0;
	// Get System All dataset identifier
	$results = ndcop_dataset_api_get_dataset('',0,'');

	foreach ($results as $result) {
		$identifiers[$result->serialno] = $result->nid;
		$count++;
	}
	$dataset_nid = ( !empty($identifiers[$id]) ) ? $identifiers[$id] : 0 ;
	return $dataset_nid;
}

/**
 * 檢查欄位值是否為空.
 * @return if is set ,field's value
 *         if not set, empty string
 */
function ndcop_dataset_api_check_isset($field_value){

	return (isset($field_value)) ? $field_value : '';
}

/**
 * 驗證APIKEY與其REQUEST IP
 * @see ndcop_dataset_api_control_dataset, ckan_demoi2_controller_2
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_apikey_ip_process(){

	$headers = getallheaders();
	$request_ip = ndcop_api_get_requestip();

	if (isset($headers['Authorization'])) {
		$authorization_api_key =  $headers['Authorization'];
		$apikey_info = ndcop_api_check_apikey_info($authorization_api_key);

		if ( $apikey_info['result'] ) {
			$validation = ( in_array($request_ip, $apikey_info['publisher_ip']) ) ? TRUE : FALSE;
			if ($validation) {
				$validation_result = array('pass'=>TRUE);
			}
			else{
				$error['error']['name'][] = '來源IP不允許';
				$error['error']['__type'] = 'ER0002:來源IP不允許';
				$validation_result = array('pass'=>FALSE , 'error'=>$error);
			}
			return  $validation_result;

		}
		else{
			$error['error']['name'][] = 'API Key錯誤';
			$error['error']['__type'] = 'ER0001:API Key錯誤';
			return  array('pass'=>FALSE , 'error'=>$error);
		}

	}
	else{
		$error['error']['name'][] = 'API Key錯誤';
		$error['error']['__type'] = 'ER0001:API Key錯誤';
		return  array('pass'=>FALSE , 'error'=>$error);
	}
}

/**
 * 驗證json data中的json格式是否有誤
 * @see ndcop_dataset_api_control_dataset, ckan_demoi2_controller_2
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_json_format($ndc_opdata_format_obj){

	$check_length = json_encode($ndc_opdata_format_obj);
    //由於發生過資料集結果為["Not found"]，多一個長度檢查，以免塞空的分類
	if( count($ndc_opdata_format_obj) == 1 and strlen($check_length) > 20){

		$validation_result = array('pass'=>TRUE);
	}
	else{
		$error['error']['name'][] = 'JSON格式錯誤';
		$error['error']['__type'] = 'ER0010:JSON格式錯誤';
		$validation_result = array('pass'=>FALSE , 'error'=>$error);
	}
	return $validation_result;
}

/**
 * 驗證json data中的publisher機關和API KEY的機關是否符合
 * @see ndcop_dataset_api_control_dataset, ckan_demoi2_controller_2
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_publisher_org( $publisher, $publish_data_oid ){
	$headers = getallheaders();

	if (isset($headers['Authorization'])) {
		$authorization_api_key =  $headers['Authorization'];
		$apikey_info = ndcop_api_check_apikey_info($authorization_api_key);

		if ( $apikey_info['result'] ) {

			if( !empty($publish_data_oid) ){
				$pos = strpos($publish_data_oid, $apikey_info['publisher_oid']);
				$validation = ($pos !== false) ? TRUE : FALSE ;

				if ($validation) {
					$validation_result = array('pass'=>TRUE);
				}
				else{
					$error['error']['name'][] = '資料集提供機關錯誤:publisherOID='.$publish_data_oid;
					$error['error']['__type'] = 'ER0033:資料集提供機關錯誤';
					$validation_result = array('pass'=>FALSE , 'error'=>$error);
				}
				return $validation_result;
			}
			else{

				$error['error']['name'][] = '資料集提供機關錯誤:未填寫publisherOID';
				$error['error']['__type'] = 'ER0033:資料集提供機關錯誤';
				return  array('pass'=>FALSE , 'error'=>$error);
			}

		}
		else{
			$error['error']['name'][] = 'API Key錯誤';
			$error['error']['__type'] = 'ER0001:API Key錯誤';
			return  array('pass'=>FALSE , 'error'=>$error);
		}
	}
	else{
		$error['error']['name'][] = 'API Key錯誤';
		$error['error']['__type'] = 'ER0001:API Key錯誤';
		return  array('pass'=>FALSE , 'error'=>$error);
	}
}

/**
 * 驗證介接進來的JSON DATA流程
 * 1. 詮釋資料是否已存在
 * 2. 必填欄位是否填寫
 * 3. 欄位資料型態與內容
 * 4. publisher機關是否存在於DRUPAL機關別taxonomy
 * @see ndcop_dataset_api_control_dataset, ckan_demoi2_controller_2
 * @param $action: (String)create;update
 * @param $data: NDCOPDATA規範的 API形式的json data string
 * @param $modify_id: 更新/刪除詮釋資料寫在URL的id
 * @return (Array)pass: Boolean ; error: Array
 */
function ndcop_api_validate_api_in_data_process($action = '', $publisher_apikey ='', $data, $modify_id = ''){
	// 通過所有驗證
	$all_validates = array();
	// 資料集是否存在
	$check_dataset_exist = FALSE;
	// 必填欄位是否填寫
	$check_required_field = array();
	// 欄位資料型態與內容
	$check_field_content_and_type = array();
	// publisher機關是否存在於DRUPAL機關別taxonomy
	$check_drupal_taxonomy_org_exist = FALSE;
	// comment commentId欄位是否正確
	$check_commentId = FALSE;
	// JSON DATA Object
	$ndc_opdata_format_obj = json_decode( $data );
	// 判斷API IN的資料集是否存在於平台上
	// 在確認資料正確性流程前，已通過APIKey和IP驗證，確保$headers['Authorization']正確
	$headers = getallheaders();
	$authorization_api_key = (isset($headers['Authorization']) and !empty($headers['Authorization']))? $headers['Authorization'] : $publisher_apikey ;
		$apikey_info = ndcop_api_check_apikey_info($authorization_api_key);
		$check_identifier = ($action == 'create') ? $ndc_opdata_format_obj->identifier : $modify_id ;
		$ndcop_id = ndcop_api_get_check_apiin_identifier($check_identifier, $apikey_info, $ndc_opdata_format_obj->publisherOID);
		$check_dataset_exist = ( !empty($ndcop_id) ) ? TRUE : FALSE ;
        // 新增資料集
        // 檢查傳入之 gov_resource_type 是否符合30種類型
        $check_gov_resource_type = ndcop_api_validate_gov_resource_type($ndc_opdata_format_obj);
        // 驗證identifier是否為unicode
        $identifierIsUnicode = ndcop_api_validate_identifierIsUnicode($ndc_opdata_format_obj);
            if($action == 'create'){
            // 驗證進來的資料集名稱是否重複
            $datasetTitleIsNotDuplicated = ndcop_api_checkTitleIsDuplicated($ndc_opdata_format_obj, '');
                if( !$check_dataset_exist ){
            // id不存在於政府資料開放平臺，可以新增
            // 判斷欲必填欄位是否填寫
                    $check_required_field = ndcop_api_validate_required_field( $ndc_opdata_format_obj );

        // 判斷欄位資料型態與內容
                    $check_field_content_and_type = ndcop_api_validate_field_content( $ndc_opdata_format_obj );

        // 判斷外部json data中的publisher機關是否存在於DRUPAL機關別taxonomy
                    $check_drupal_taxonomy_org_exist = ndcop_api_validate_drupal_taxonomy_org( $ndc_opdata_format_obj);
            //判斷comment commentReplyId欄位是否正確
                    $check_commentId = ndcop_api_validate_commentId($ndc_opdata_format_obj);
                }

        // 通過所有的驗證
                if( !$check_dataset_exist and $check_drupal_taxonomy_org_exist and $check_commentId and $check_required_field['pass'] and $check_field_content_and_type['pass'] and $check_gov_resource_type['pass'] and $identifierIsUnicode['pass'] and $datasetTitleIsNotDuplicated['pass']){
            // 確認可以開始新增
                    $all_validates['pass'] = TRUE;

                }
        // 未通過驗證
                else{

                    $all_validates['pass'] = FALSE;
                    if ($check_dataset_exist) {
                        $error['error']['name'][] = '欲新增的資料集已存在';
                        $error['error']['__type'] = 'ER0040:欲新增的資料集已存在';
                        $all_validates['error'] = $error;
                    }
                    elseif (!$check_drupal_taxonomy_org_exist) {
                // 這個ERROR不存在於規範，屬於此平台的ERROR
                        $error['error']['name'][] = '政府資料開放平臺不存在介接資料中的publisherOID( '.$ndc_opdata_format_obj->publisherOID.' )對應之機關或單位';
                        $error['error']['__type'] = '';
                        $all_validates['error'] = $error;
                    }
                    elseif (!$check_required_field['pass']) {
                        $all_validates['error'] = $check_required_field['error'];
                    }
                    elseif (!$check_field_content_and_type['pass']) {
                        $all_validates['error'] = $check_field_content_and_type['error'];
                    }
                    else if(!$check_commentId){
                        $error['error']['name'][] = 'commentId或commentReplyId填寫錯誤';
                        $error['error']['__type'] = 'comment錯誤';
                        $all_validates['error'] = $error;
                    }else if(!$check_gov_resource_type['pass']){
                        $all_validates['error'] = $check_gov_resource_type['error'];
                    }elseif(!$identifierIsUnicode['pass']){
                        $all_validates['error'] = $identifierIsUnicode['error'];
                    }
                    elseif(!$datasetTitleIsNotDuplicated['pass']){
                        $all_validates['error'] = $datasetTitleIsNotDuplicated['error'];
                    }
				}
				return $all_validates;

	} // 新增資料集End

	// 修改資料集
	elseif($action == 'update'){
        if( $check_dataset_exist ){
            // 存在於政府資料開放平臺，可以修改
            // 驗證進來的資料集名稱是否重複
            $datasetTitleIsNotDuplicated = ndcop_api_checkTitleIsDuplicated($ndc_opdata_format_obj, $ndc_opdata_format_obj->{'identifier'});
			// 判斷欲必填欄位是否填寫
			$check_required_field = ndcop_api_validate_required_field( $ndc_opdata_format_obj );
	    // $check_required_field['pass'] = TRUE;

	    // 判斷欄位資料型態與內容
			$check_field_content_and_type = ndcop_api_validate_field_content( $ndc_opdata_format_obj );

	    // 判斷外部json data中的publisher機關是否存在於平台機關別
			$check_drupal_taxonomy_org_exist = ndcop_api_validate_drupal_taxonomy_org( $ndc_opdata_format_obj);
			//判斷comment commentReplyId欄位是否正確
			$check_commentId = ndcop_api_validate_commentId($ndc_opdata_format_obj,$ndcop_id);
		}
		// 通過所有的驗證
		if($check_commentId and $check_dataset_exist and $check_drupal_taxonomy_org_exist and $check_required_field['pass'] and $check_field_content_and_type['pass']  and $check_gov_resource_type['pass'] and $identifierIsUnicode['pass'] and $datasetTitleIsNotDuplicated['pass']){
			// 確認可以開始修改
			$all_validates['pass'] = TRUE;
			$all_validates['dataset_nid'] = $ndcop_id;
		}else{
			$all_validates['pass'] = FALSE;

			if (!$check_dataset_exist) {
				$error['error']['name'][] = '欲修改的資料集不存在';
				$error['error']['__type'] = 'ER0050:欲修改的資料集不存在';
				$all_validates['error'] = $error;
			}
			elseif (!$check_drupal_taxonomy_org_exist) {
				$error['error']['name'][] = '政府資料開放平臺不存在介接資料中的publisherOID( '.$ndc_opdata_format_obj->publisherOID.' )對應之機關或單位';
				$error['error']['__type'] = '';
				$all_validates['error'] = $error;
			}
			elseif (!$check_required_field['pass']) {
				$all_validates['error'] = $check_required_field['error'];
			}
			elseif (!$check_field_content_and_type['pass']) {
				$all_validates['error'] = $check_field_content_and_type['error'];
			}
			elseif (!$check_commentId) {
				$error['error']['name'][] = 'commentId填寫錯誤';
				$error['error']['__type'] = 'comment錯誤';
				$all_validates['error'] = $error;
			}else if(!$check_gov_resource_type['pass']){
                        $all_validates['error'] = $check_gov_resource_type['error'];
            }else if(!$identifierIsUnicode['pass']){
                        $all_validates['error'] = $identifierIsUnicode['error'];
            }
            elseif(!$datasetTitleIsNotDuplicated['pass']){
                $all_validates['error'] = $datasetTitleIsNotDuplicated['error'];
            }
		}

		return $all_validates;
	}

}

/**
 * 驗證必填欄位未填
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format object
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_required_field( $ndc_opdata_format_obj ){

	$required_empty = '';
	$flag = 0;
	$show_field_info = ndcop_api_generate_dataset_field_info();
	foreach ($show_field_info as $field_name => $info) {
		if( $info['required'] == 1 && empty($ndc_opdata_format_obj->{$field_name}) ){
			$flag = 1;
			$required_empty .= ($field_name.' 未填, ');
		}
	}

    // 檢查資料資源
	if(isset($ndc_opdata_format_obj->distribution) and count($ndc_opdata_format_obj->distribution) > 0){
    	// 取得資料資源欄位必填資訊
		$show_resource_field_info = ndcop_api_generate_resource_field_info();
		foreach ($ndc_opdata_format_obj->distribution as $distribution) {
			foreach ($show_resource_field_info as $field_name => $info) {
				if( $info['required'] == 1 && empty($distribution->{$field_name}) ){
					$flag = 1;
					$required_empty .= ($field_name.' 未填, ');
				}
			}
		}
	}
	else{
		$flag = 1;
		$required_empty .= ('distribution 未填, ');
	}

	if($flag == 1){
		$error['error']['name'][] = '必填欄位未填:'.$required_empty;
		$error['error']['__type'] = 'ER0020:必填欄位未填';
		$validation_result = array('pass'=>FALSE , 'error'=>$error);
	}
	else{
		$validation_result = array('pass'=>TRUE);
	}
	return $validation_result;
}

/**
 * 驗證欄位資料型態與內容
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format object
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_field_content( $ndc_opdata_format_obj ){

	// 驗證資料集分類編號
	$check_categoryCode = FALSE;

	$check_categoryCode = ndcop_api_validate_is_catagoryCode($ndc_opdata_format_obj->categoryCode);

	$check_organizationContactEmail = ndcop_api_validate_email_type( $ndc_opdata_format_obj->organizationContactEmail );

	$check_contactEmail = ndcop_api_validate_email_type( $ndc_opdata_format_obj->publisherContactEmail );

	$check_numberOfData = ndcop_api_validate_postive_int( $ndc_opdata_format_obj->numberOfData );

	if( isset($ndc_opdata_format_obj->issued) and !empty($ndc_opdata_format_obj->issued)){
		$check_issued = ndcop_api_validate_datetime_type($ndc_opdata_format_obj->issued);
	}
	if( isset($ndc_opdata_format_obj->modified) and !empty($ndc_opdata_format_obj->modified)){
		$check_modified = ndcop_api_validate_datetime_type($ndc_opdata_format_obj->modified);
	}

	// 判斷資料項目(distributions)(resources) resource ID是否重複
	$check_duplicate_resoureid = ndcop_api_validate_duplicate_resourceid( $ndc_opdata_format_obj->distribution );

	// 驗證資料集分類編號
	if ( !$check_categoryCode ) {
		$error['error']['name'][] = '資料集分類錯誤:無此分類categoryCode='. $ndc_opdata_format_obj->categoryCode;
		$error['error']['__type'] = 'ER0031:資料集分類錯誤';
		$validation_result = array('pass'=>FALSE , 'error'=>$error);
	}
	else{
		// 驗證欄位資料型態錯誤
		$field_type_error_msg = '';
		if( !$check_organizationContactEmail ){
			$field_type_error_msg .= 'organizationContactEmail:'. $ndc_opdata_format_obj->organizationContactEmail;
		}
		if( !$check_contactEmail ){
			$field_type_error_msg .= 'contactEmail:'. $ndc_opdata_format_obj->publisherContactEmail;
		}
		if( !empty($ndc_opdata_format_obj->numberOfData) and !$check_numberOfData ){
			$field_type_error_msg .= 'numberOfData:'. $ndc_opdata_format_obj->numberOfData. '不為整數;';
		}
		if( isset($check_issued) and !$check_issued ){
			$field_type_error_msg .= 'issued:'. $ndc_opdata_format_obj->issued. '日期格式請參照資料集詮釋資料標準規範建議範例;';
		}
		if( isset($check_modified) and !$check_modified ){
			$field_type_error_msg .= 'modified:'. $ndc_opdata_format_obj->modified. '日期格式請參照資料集詮釋資料標準規範建議範例;';
		}
		if( !$check_duplicate_resoureid['pass'] ){
			$field_type_error_msg .= 'distribution:'. $check_duplicate_resoureid['error'];
		}
		if(!empty($field_type_error_msg)){
			$error['error']['name'][] = '欄位資料型態錯誤 '.$field_type_error_msg;
			$error['error']['__type'] = 'ER0030:欄位資料型態錯誤';
			$validation_result = array('pass'=>FALSE , 'error'=>$error);
		}
		else{
			// 通過驗證資料集分類編號、通過驗證欄位資料型態
			$validation_result = array('pass'=>TRUE);
		}
	}


	return $validation_result;
}

/**
 * 驗證資料項目(distributions)(resources) resource ID是否重複
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format object
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_duplicate_resourceid( $distributions ){

	$validate_result = array();
	$resource_id_list = array();
	$duplicate_msg = '';

	if(count($distributions) > 1){
		foreach ($distributions as $distribution) {
			$resource_id_list[] = $distribution->resourceID;
		}
		$resource_id_list_count = array_count_values($resource_id_list);
		// 是否有計數大於1的ResourceID
		$check_count_result = array_filter($resource_id_list_count, function($resource_count, $key){
			return ( $resource_count > 1 );
		}) ;
		foreach ($check_count_result as $resource_id => $resource_count) {
			$duplicate_msg .= ($resource_id. '重複個數'. $resource_count.'; ');
		}

		if( count($check_count_result) > 0 ){
			$validation_result['pass'] = FALSE;
			$validation_result['error'] = 'resourceID重複'. $duplicate_msg;
		}
		else{
			$validation_result['pass'] = TRUE;
		}
	}
	else{
		$validation_result['pass'] = TRUE;
	}

	return $validation_result;
}

/**
 * 驗證外部json data中的publisher機關是否存在於DRUPAL機關別taxonomy
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format_obj 介接進來的資料
 * @return boolean
 */
function ndcop_api_validate_drupal_taxonomy_org( $ndc_opdata_format_obj ){

	$tid = ndcop_api_get_ndc_org($ndc_opdata_format_obj->publisherOID, $ndc_opdata_format_obj->publisher, $ndc_opdata_format_obj->publisherOrgCode );
	return ($tid > 0) ? TRUE : FALSE ;
}

/**
 *判斷comment commentId欄位是否正確
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format_obj 介接進來的資料
 * @return boolean
 */
function ndcop_api_validate_commentId($ndc_opdata_format_obj,$nid=''){
    $ccid_list=array();
    $reply_list=array();
    foreach($ndc_opdata_format_obj->Comments as $key=> $comment) {
        $cpid = $comment->commentReplyId;
        $ccid = $comment->commentId;
        if($nid!=''){
            if(ndcpo_api_comment_exist($nid,$ccid)){
                if(!ndcpo_api_check_pid($nid,$ccid,$cpid))
                    return false;
            }
        }
        if($cpid!=''){
            if(in_array($cpid,$reply_list))
                return false;
            if(!in_array($cpid,$ccid_list))
                return false;
            array_push($reply_list,$ccid);

        }
        array_push($ccid_list,$ccid);
    }
    // exit;
    if(!ndcpo_api_check_array($ccid_list)){
      return false;
  }
  return TRUE;
}
/**
 *判斷該留言是否已存在
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format_obj 介接進來的資料
 * @return boolean
 */
function ndcpo_api_comment_exist($nid,$ccid){
    $result = db_query("
        select ccid.field_cid_api_value from `comment` as com
        left join field_data_field_cid_api as ccid on ccid.entity_id = com.cid
        where com.nid = :nid and ccid.field_cid_api_value = :ccid
        ",
        $arrayName = array(':nid' => $nid,':ccid' => $ccid)
        );
    return ($result->rowCount()>0)?true:false;
}


/**
 *判斷該留言PID是否正確
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format_obj 介接進來的資料
 * @return boolean
 */
function ndcpo_api_check_pid($nid,$ccid,$cpid){
    $result = db_query("
        select ccid.field_cid_api_value from `comment` as com
        left join field_data_field_cid_api as ccid on ccid.entity_id = com.cid
        left join field_data_field_pid_api as cpid on cpid.entity_id = com.cid
        where com.nid = :nid and ccid.field_cid_api_value = :ccid and cpid.field_pid_api_value = :cpid
        ",
        $arrayName = array(':nid' => $nid,':ccid' => $ccid ,':cpid' => $cpid)
        );
    return ($result->rowCount()>0)?true:false;
}


/**
 *判斷陣列是否按順序排列，以及陣列是否有重複的值
 * @see ndcop_api_validate_api_in_data_process
 * @param ndc_opdata_format_obj 介接進來的資料
 * @return boolean
 */
function ndcpo_api_check_array($arr){
	$default = $arr;
	sort($arr);
	foreach($arr as $key=>$value)
	{
		if($value!=$default[$key])
			return false;
	}
	$arr_unique = array_unique($arr);
	if ( count($arr_unique) != count($arr) ){
		return false;
	}
	return true;
}

/**
 * 驗證欄位資料型態錯誤-電子郵件
 * @see
 * @param email
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_email_type( $email ){
	$pattern = "/(^[a-zA-Z\d.\-_]+@[a-zA-Z\d.-]+\.[a-zA-Z]{2,4},?)+/";
	if (preg_match($pattern,$email)){
		return TRUE;
	}
	else{
		return FALSE;
	}
}



/**
 * 驗證欄位資料型態錯誤-日期時間
 * @see
 * @param $datetime
 * @return (Array) pass=>boolean, error=>(array)
 */
function ndcop_api_validate_datetime_type( $datetime ){
	$pattern = "/(^[a-zA-Z\d.\-_]+@[a-zA-Z\d.-]+\.[a-zA-Z]{2,4},?)+/";
	$pattern = "/^(\d{4}\-(0?[1-9]|1[012])\-(0?[1-9]|[12][0-9]|3[01]))|(\d{4}\-(0?[1-9]|1[012])\-(0?[1-9]|[12][0-9]|3[01])\s(([0-1]?[0-9])|([2][0-3])):([0-5]?[0-9])(:([0-5]?[0-9])))$/";
	if (preg_match($pattern, $datetime)){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

function ndcop_api_validate_gov_resource_type($ndc_opdata_format_obj)
{

    $isValidate = db_query("SELECT ttd.name
                     FROM taxonomy_term_data ttd
                     WHERE vid = 25")->fetchAll(PDO::FETCH_ASSOC);
    $govResourceTypeErr = '';
    $flag = 0;

    $validateAry = array();
    foreach ($isValidate as $format) {
        $validateAry[] = $format['name'];
    }
    foreach($ndc_opdata_format_obj->{'distribution'} as $item){
        $item->{'format'} = strtoupper($item->{'format'});
        if(!in_array($item->{'format'}, $validateAry)){
            $flag = 1;
            $govResourceTypeErr .= '不應有【' . $item->{'format'} . '】';
        }
    }
    if($flag == 1){
        $error['error']['name'][] = '檔案格式填寫錯誤:'.$govResourceTypeErr;
        $error['error']['__type'] = 'ER0070:檔案格式填寫錯誤';
        $validation_result = array('pass'=>FALSE , 'error'=>$error);
    }
    else{
        $validation_result = array('pass'=>TRUE);
    }
    return $validation_result;
}

function ndcop_api_validate_identifierIsUnicode($ndc_opdata_format_obj)
{
    $isUnicode = preg_match("/\p{Han}+/u", $ndc_opdata_format_obj->{'identifier'});
    $identifierTypeErr = '';
    $flag = 0;
    if($isUnicode === 1){
        $flag = 1;
        $identifierTypeErr .= '不應有中文字元';
    }

    if($flag == 1){
        $error['error']['name'][] = 'identifier格式錯誤:'.$identifierTypeErr;
        $error['error']['__type'] = 'ER0071:identifier格式錯誤';
        $validation_result = array('pass'=>FALSE , 'error'=>$error);
    }
    else{
        $validation_result = array('pass'=>TRUE);
    }
    return $validation_result;
}

function ndcop_api_checkTitleIsDuplicated($ndc_opdata_format_obj, $identifier)
{
    $query = db_select('node', 'n');
    $query->leftJoin('field_data_field_oid', 'ffo', 'ffo.entity_id = n.nid');
    $query->condition('n.type', 'metadataset', '=');
    $db_and = db_and();
    $db_and->condition('ffo.field_oid_value', $ndc_opdata_format_obj->{'publisherOID'}, '=');
    $db_and->condition('n.title', $ndc_opdata_format_obj->{'title'}, '=');
    if(!empty($identifier)){
        $query->innerJoin('field_data_field_serialno', 'ffsn', 'ffsn.entity_id = n.nid');
        $db_and->condition('ffsn.field_serialno_value', $identifier, '!=');
    }
    $query->fields('n', array('title'));
    $query->condition($db_and);
    $isDuplicated = $query->execute()->fetchAssoc();
    $titleErr = '';
    $flag = 0;
    if($isDuplicated){
        $flag = 1;
        $titleErr .= $isDuplicated['title'];
    }

    if($flag == 1){
        $error['error']['name'][] = '資料集名稱【'.$titleErr.'】重複';
        $error['error']['__type'] = 'ER0072:資料集名稱重複';
        $validation_result = array('pass'=>FALSE , 'error'=>$error);
    }
    else{
        $validation_result = array('pass'=>TRUE);
    }
    return $validation_result;
}
