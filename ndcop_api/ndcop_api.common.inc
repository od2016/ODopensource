<?php

/**
 * @file
 * API常用Help Functions.
 * TODO:
 */

/**
 * 取得categoryCode 的tid集合.
 */

function ndcop_dataset_api_get_category_tids($offset, $limit){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'taxonomy_term')
  ->entityCondition('bundle', array('data_catalog'))
  ->propertyOrderBy('weight');

  if ($limit) {
    $query->range($offset, $limit);
  }
  $result = $query->execute();
  $tids = array_keys($result['taxonomy_term']);
  return $tids;
}

/**
 * 取得tag 的tid集合.
 */
function ndcop_dataset_api_get_tag_tids($offset, $limit){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'taxonomy_term')
  ->entityCondition('bundle', array('tags'))
  ->propertyOrderBy('weight');

  if ($limit) {
    $query->range($offset, $limit);
  }
  $result = $query->execute();
  $tids = array_keys($result['taxonomy_term']);
  return $tids;
}

/**
* 取得新 tag (dataset_tags) 的集合.
*/
function ndcop_dataset_api_get_dataset_tags($offset, $limit){
  $query = db_select('field_data_field_dataset_tags', 'ffdt');
  $query->addField('ffdt', 'field_dataset_tags_tag');
  $query->condition('ffdt.field_dataset_tags_tag', '', '!=');
  $query->groupBy('ffdt.entity_id');
  if($limit){
    $query->range($offset, $limit);
  }
  $results = $query->execute()->fetchAll(PDO::FETCH_COLUMN);
  return $results;
}


/**
 * 取得dataset 的identifier集合.
 */
function ndcop_dataset_api_get_dataset($modified, $offset, $limit, $oid = ''){
	$query = db_select('node', 'node');
	$query->leftJoin(
   'field_data_field_serialno',
   'field_serialno',
   'field_serialno.entity_id = node.nid AND field_serialno.bundle = \'metadataset\' AND field_serialno.deleted = 0 '
   );
	$query->leftJoin(
   'field_data_field_version_update_time',
   'field_version_update_time',
   'field_version_update_time.entity_id = node.nid AND field_version_update_time.bundle = \'metadataset\' AND field_version_update_time.deleted = 0 '
   );
  $query->leftJoin(
    'field_data_field_orgfullname',
    'field_orgfullname',
    'field_orgfullname.entity_id = node.nid AND field_orgfullname.bundle = \'metadataset\' AND field_orgfullname.deleted = 0 '
    );
  $query->leftJoin(
    'field_data_field_orgoid',
    'field_orgoid',
    'field_orgoid.entity_id = field_orgfullname.field_orgfullname_tid AND field_orgoid.bundle = \'org\' AND field_orgoid.deleted = 0 '
    );

  $query->condition('node.status', 1);
  $query->condition('node.type', array('metadataset'), 'IN' );

	// 參數 modified
  if( !empty($modified)){
    $modified = strtotime($modified);
    $query->condition('field_version_update_time.field_version_update_time_value', $modified, '>=' );
  }

  $query->addField('field_serialno','field_serialno_value','serialno');
  $query->addField('node','nid','nid');

	// 參數 limit
  if ($limit) {
    $query->range($offset, $limit);
  }

  // 參數 oid(規範沒有)
  if( !empty($oid)){
    $query->condition('field_orgoid.field_orgoid_value', $oid );
  }

  $results = $query->execute();
  return $results;
}

/**
 * 取得dataset 的API IN identifier集合.
 * @param $oid: 機關(單位) OID
 */
function ndcop_api_get_apiin_dataset_id( $oid = '' ){

  $id_set = array();
  $query = db_select('field_data_'.NDCOP_API_APIIN_IDENTIFIER, NDCOP_API_APIIN_IDENTIFIER);

  $query->leftJoin(
    'field_data_field_orgfullname',
    'field_orgfullname',
    'field_orgfullname.entity_id = '.NDCOP_API_APIIN_IDENTIFIER.'.entity_id AND field_orgfullname.bundle = \'metadataset\' AND field_orgfullname.deleted = 0 '
    );
  $query->leftJoin(
    'field_data_field_orgoid',
    'field_orgoid',
    'field_orgoid.entity_id = field_orgfullname.field_orgfullname_tid AND field_orgoid.bundle = \'org\' AND field_orgoid.deleted = 0 '
    );

  $query->addField(NDCOP_API_APIIN_IDENTIFIER, NDCOP_API_APIIN_IDENTIFIER.'_value','apiin_id');

  if( !empty($oid) ){
    $query->condition('field_orgoid.field_orgoid_value', db_like($oid).'%', 'LIKE' );
  }

  $results = $query->execute();

  foreach ($results as $result) {
    $id_set[] = $result->apiin_id;
  }

  return $id_set;
}

/**
 * 根據vocabulary機器碼取得taxomomy array.
 * @return Array
 */
function ndcop_api_get_taxonomy_term_names($vocabulary_name){
	$taxonomy_term = array();
	$vocabulary_obj =  taxonomy_vocabulary_machine_name_load($vocabulary_name);
	foreach(taxonomy_get_tree($vocabulary_obj->vid) as $term) {
		$taxonomy_term[$term->name] = $term->tid ;
	}
	return $taxonomy_term;
}

/**
 * 切割關鍵字(文字欄位).
 * @return Array
 */
function ndcop_dataset_api_split_text_keywords($keywords){
	$split_keywords = array();
	$keyword_set = array();
		// echo $key.': '.$keywords."<br>";
	// 非空字串
	if(!empty($keywords)){
		if( strpos($keywords, ',') ){
			$keyword_set = explode(',', $keywords);
		}
		elseif ( strpos($keywords, '，') ){
			$keyword_set = explode('，', $keywords);
		}
		elseif ( strpos($keywords, '、') ){
			$keyword_set = explode('、', $keywords);
		}
		elseif ( strpos($keywords, '；') ){
			$keyword_set = explode('；', $keywords);
		}
		elseif ( strpos($keywords, ';') ){
			$keyword_set = explode(';', $keywords);
		}
		// 只有一個關鍵字
		else{
			// keyeord非空
			if (!empty($keywords) and ($keywords !== ' ') ){
        $split_keywords[] = check_plain( trim($keywords) );
      }
    }

		// 可切割關鍵字
    if (!empty($keyword_set)) {
			// print_r($keyword_set);
     foreach ($keyword_set as $key => $keyword) {

				// keyeord非空
      if (!empty($keyword) and ($keyword !== ' ') ){
       $split_keywords[] = check_plain( trim($keyword) );
     }
   }

 }
}
return $split_keywords;
}

/**
 * 切割關鍵字(term reference欄位).
 * @return Array
 */
function ndcop_dataset_api_split_term_ref_keywords($keywords){

	$split_keywords = array();

	foreach ($keywords as $keyword ) {
		$keyword_term = taxonomy_term_load( $keyword['tid'] );
		$split_keywords[] = $keyword_term->name;
	}

	return $split_keywords;
}

/**
 * 取得REQUEST IP
 * @see ndcop_api_validate_apikey_ip_process
 * @return REQUEST IP
 */
function ndcop_api_get_requestip(){
	if (!empty($_SERVER['HTTP_CLIENT_IP'])){
    $ip = $_SERVER['HTTP_CLIENT_IP'];
  }
  else if (!empty($_SERVER['HTTP_X_FORWARDED_FOR'])){
    $ip = $_SERVER['HTTP_X_FORWARDED_FOR'];
  }
  else{
    $ip = $_SERVER['REMOTE_ADDR'];
  }
  return $ip;
}

/**
 * 根據APIKEY取出其對應IP、資料提供機關名稱
 * @see ndcop_api_validate_apikey_ip_process
 * @param $authorization_api_key: HTTP Header Authorization所取得的APIKEY
 * @return APIKEY和其對應IP
 */
function ndcop_api_check_apikey_info($authorization_api_key){

  $apikey_info = array();

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'taxonomy_term')
  ->entityCondition('bundle', array('org'))
  ->fieldCondition('field_publisher_api_key', 'value', $authorization_api_key, '=');

  $result = $query->execute();
  $tid_set = array_keys($result['taxonomy_term']);

  // NDCOP平臺上存在此APIKey相對應的機關(或單位)
  if( !empty($tid_set ) and ( count($tid_set ) == 1 ) ){
    $apikey_related_org = taxonomy_term_load($tid_set[0]);
    $apikey_info['result'] = TRUE;
    $apikey_info['publisher_tid'] = $tid_set[0];
    $apikey_info['publisher_name'] = $apikey_related_org->name;
    $apikey_info['publisher_oid'] = $apikey_related_org->field_orgoid[LANGUAGE_NONE][0]['value'];
    // 多個IP用逗號分隔
    $pos = strpos($apikey_related_org->field_publisher_ip[LANGUAGE_NONE][0]['value'], ',');
    if ($pos !== false) {
      $apikey_info['publisher_ip'] = array_map('trim', explode(',', $apikey_related_org->field_publisher_ip[LANGUAGE_NONE][0]['value'] ) );
    }
    else{
      $apikey_info['publisher_ip'] = array(trim($apikey_related_org->field_publisher_ip[LANGUAGE_NONE][0]['value']) );
    }
  }
  // NDCOP平臺上不存在此APIKey相對應的機關(或單位)或找到一個以上
  else{
    $apikey_info['result'] = FALSE;
  }
  return $apikey_info;
}

/**
 * 取得政府資料開放平臺的identifier
 * @see ndcop_api_validate_data_in_process
 * @param $api_in_identifier: 介接進來的identifier
 * @return
 */
function ndcop_api_get_ndcop_identifier($api_in_identifier){
	if($api_in_identifier == NDCOP_API_TEST_API_IN_ID){
		$results['identifier'] = NDCOP_API_TEST_API_NDCOP_ID;
		return $results['identifier'];
	}
	else{
		return  $results;
	}
}

/**
 * API IN的identifier是否存在於政府資料開放平臺
 * @see ndcop_api_validate_data_in_process
 * @param $api_in_identifier: 介接進來的identifier
 * @param $apikey_info: 介接進來的機關資訊
 * @param $api_in_data_oid: 介接進來的publisherOID (若為DELETE則為空)
 * @return
 */
function ndcop_api_get_check_apiin_identifier($api_in_identifier, $apikey_info, $api_in_data_oid = ''){
  // 若為新增或更新，使用介接平臺送過來的資料文件中之identifier和publisherOID，尋找NDC平臺內是否存在該資料文件，因為在此之前已比對過外部json data中的oid(publisherOID)和API KEY機關資訊中的oid，所以該份資料文件的publisherOID要當作過濾條件之一
  // DB Access: Get dataset identifier By tid of category_code
  $node_query = new EntityFieldQuery();

  $node_query->entityCondition('entity_type', 'node')
  ->entityCondition('bundle', 'metadataset')
      ->fieldCondition('field_dataset_in_id_g', 'value', $api_in_identifier); // APIIN的identifier

  // Create, Update
      if( !empty($api_in_data_oid) ){
        $node_query->fieldCondition('field_oid', 'value', $api_in_data_oid  );
      }
  // Delete
      else{
        $node_query->fieldCondition('field_oid', 'value', $apikey_info['publisher_oid'].'%', 'like'  );
      }

      $result = $node_query->execute();
      if ( isset($result['node']) ) {
        if( count($result['node']) == 1 ){
          $dataset_items_nid = array_keys($result['node']);
          return $dataset_items_nid[0];
        }
        else{
          throw new Exception('介接進來的機關於政府資料開放平臺上有兩筆以上相同的API IN identifier');
        }
      }

    }

/**
 *  顯示國發會API規範訊息
 *  @param $identifier:
 *  @param $result: json string
 *  @return echo json_encode($output_msg): json string
 */
function ndcop_api_show_result_msg($identifier, $result , $post_data_json_str = '' ){
	$create_error = ndcop_api_check_json_error($result);
  $requestip = ndcop_api_get_requestip();
  $output = array();

  	// Return success false
  if( $create_error['check_result'] ){

    $output['success'] = false;
    $output['error']['identifier'] = $identifier;
    $output['error']['error_type'] = $create_error['error_type'];
    $output['error']['message'] = $create_error['error_msg'];
    $err_type = explode(":", $create_error['error_type']);
    ndcop_api_dataset_log( $_SERVER['REQUEST_METHOD'], $requestip, $_SERVER['REQUEST_URI'], $post_data_json_str, 0, $err_type[0], $output['error']['message'] , $_SERVER['REQUEST_TIME']);
  }

    // Return success true
  else{
    $output['success'] = TRUE;
    $output['result'] = array('identifier'=>$identifier);

      // ndcop_dataset_api Log
    ndcop_api_dataset_log( $_SERVER['REQUEST_METHOD'], $requestip, $_SERVER['REQUEST_URI'], $post_data_json_str, 1, '', $output['result'] , $_SERVER['REQUEST_TIME']);
  }

  drupal_json_output($output);

    // LOG回傳的的JSON DATA
  watchdog('ndcop_dataset_api','DRUPAL return results: %json',
   array('%json' => json_encode($output), WATCHDOG_DEBUG));
}

/**
 *  Log 跨機關介接Harvest資訊
 *  @param $identifier:
 *  @param $result: json string
 *  @return echo json_encode($output_msg): json string
 */
function ndcop_api_log_harvest_result($harvest_action, $harvest_api_sru, $harvest_api_accesspoint ,$identifier, $result , $post_data_json_str = '' ){

  $create_error = ndcop_api_check_json_error($result);
  $output = array();

  if( $create_error['check_result'] ){

    $output['success'] = false;
    $output['error']['identifier'] = $identifier;
    $output['error']['error_type'] = $create_error['error_type'];
    $output['error']['message'] = $create_error['error_msg'];
    $err_type = explode(":", $create_error['error_type']);
    ndcop_api_dataset_log( $harvest_action, $harvest_api_sru, $harvest_api_accesspoint, $post_data_json_str, 0, $err_type[0], $output['error']['message'] , time() );
  }

    // Return success true
  else{
    $output['success'] = TRUE;
    $output['result'] = array('identifier'=>$identifier);

    ndcop_api_dataset_log( $harvest_action, $harvest_api_sru, $harvest_api_accesspoint, $post_data_json_str, 1, '', $output['result'] , time() );
  }

  drupal_json_output($output);

    // LOG回傳的的JSON DATA
  watchdog('ndcop_dataset_api','DRUPAL return results: %json',
   array('%json' => json_encode($output), WATCHDOG_DEBUG));
}

/**
 *  檢查回傳的JSON是否有error發生
 *  @see ndcop_api_show_result_msg
 *  @param $json_str: json string
 */
function ndcop_api_check_json_error($json_str){
  $jsondecode_result = json_decode($json_str,true);

  if( isset( $jsondecode_result['error'] ) ){
    $error_msg = '';
    foreach ($jsondecode_result['error']['name'] as $key => $msg) {
      $error_msg .= $msg;
    }
    return array('check_result'=>TRUE, 'error_type'=>$jsondecode_result['error']['__type'], 'error_msg'=>$error_msg);
  }
  else{
    return array('check_result'=>FALSE);
  }
}

/**
 * 產生資料集詮釋資料欄位相關資訊的陣列。例如:必填、權重
 * @see template_preprocess_ckan_demoi2_custompage2_template
 * @see ckan_demoi2_validate_required_field
 * @return (Array)$ndcopdata_api_format
 */
function ndcop_api_generate_dataset_field_info(){

  $ndcopdata_api_format = array();
  $ndcopdata_api_format['identifier'] = array(
    'required' => 1,
    'weight' => 1,
    );
  $ndcopdata_api_format['categoryCode'] = array(
    'required' => 1,
    'weight' => 2,
    );
  $ndcopdata_api_format['title'] = array(
    'required' => 1,
    'weight' => 3,
    );
  $ndcopdata_api_format['description'] = array(
    'required' => 1,
    'weight' => 4,
    );
  $ndcopdata_api_format['fieldDescription'] = array(
    'required' => 1,
    'weight' => 5,
    );
  $ndcopdata_api_format['type'] = array(
    'required' => 1,
    'weight' => 6,
    );
  $ndcopdata_api_format['publisher'] = array(
    'required' => 1,
    'weight' => 7,
    );
  $ndcopdata_api_format['numberOfData']= array(
    'required' => 0,
    'weight' => 8,
    );
  $ndcopdata_api_format['temporalCoverage'] = array(
    'required' => 0,
    'weight' => 9,
    );
  $ndcopdata_api_format['accrualPeriodicity'] = array(
    'required' => 1,
    'weight' => 10,
    );
  $ndcopdata_api_format['modified'] = array(
    'required' => 0,
    'weight' => 11,
    );
  $ndcopdata_api_format['license'] =array(
    'required' => 1,
    'weight' => 12,
    );
  $ndcopdata_api_format['licenseURL'] = array(
    'required' => 1,
    'weight' => 13,
    );
  $ndcopdata_api_format['cost'] = array(
    'required' => 1,
    'weight' => 14,
    );
  $ndcopdata_api_format['costURL'] = array(
    'required' => 0,
    'weight' => 15,
    );
  $ndcopdata_api_format['costLaw'] = array(
    'required' => 0,
    'weight' => 16,
    );
  $ndcopdata_api_format['landingPage'] = array(
    'required' => 0,
    'weight' => 17,
    );
  $ndcopdata_api_format['characterSetCode'] = array(
    'required' => 0,
    'weight' => 18,
    );
  $ndcopdata_api_format['keyword'] = array(
    'required' => 0,
    'weight' => 19,
    );
  $ndcopdata_api_format['organization'] = array(
    'required' => 1,
    'weight' => 20,
    );
  $ndcopdata_api_format['organizationContactName'] = array(
    'required' => 1,
    'weight' => 21,
    );
  $ndcopdata_api_format['organizationContactPhone'] = array(
    'required' => 1,
    'weight' => 22,
    );
  $ndcopdata_api_format['organizationContactEmail'] = array(
    'required' => 1,
    'weight' => 23,
    );
  $ndcopdata_api_format['publisherContactName'] = array(
    'required' => 1,
    'weight' => 24,
    );
  $ndcopdata_api_format['publisherContactPhone'] = array(
    'required' => 1,
    'weight' => 25,
    );
  $ndcopdata_api_format['publisherContactEmail'] = array(
    'required' => 1,
    'weight' => 26,
    );
  $ndcopdata_api_format['publisherOID'] = array(
    'required' => 1,
    'weight' => 27,
    );
  $ndcopdata_api_format['publisherOrgCode'] = array(
    'required' => 0,
    'weight' => 28,
    );
  $ndcopdata_api_format['notes'] = array(
    'required' => 0,
    'weight' => 29,
    );
  return $ndcopdata_api_format;
}

/**
 * 產生資料集資料資源欄位相關資訊的陣列。例如:必填、權重
 * @return (Array)$ndcopdata_api_format
 */
function ndcop_api_generate_resource_field_info(){

  $ndcopdata_api_format = array();
  $ndcopdata_api_format['resourceID'] = array(
    'required' => 1,
    'weight' => 1,
    );
  $ndcopdata_api_format['resourceDescription'] = array(
    'required' => 1,
    'weight' => 2,
    );
  $ndcopdata_api_format['format'] = array(
    'required' => 1,
    'weight' => 3,
    );
  $ndcopdata_api_format['characterSetCode'] = array(
    'required' => 1,
    'weight' => 4,
    );
  return $ndcopdata_api_format;
}

/**
 * 產生資料集類型的陣列。array('原始資料'=>'rawData', '行動化應用軟體'=>'APPS', '其他'=>'TOOLS', '系統介接程式'=>'SERVICES')
 * @return (Array)
 */
function ndcop_api_generate_type_name_mapping(){

  // 資料集類型
  $type_name = array('原始資料'=>'rawData', '行動化應用軟體'=>'APPS', '其他'=>'TOOLS', '系統介接程式'=>'SERVICES');
  return $type_name;
}

/**
 * 產生GUID
 * @return GUID
 */
function ndcop_api_get_guid(){
  if (function_exists('com_create_guid')){
    return com_create_guid();
  }else{
        mt_srand((double)microtime()*10000);//optional for php 4.2.0 and up.
        $charid = md5(uniqid(rand(), true));
        $hyphen = chr(45);// "-"

        $uuid = substr($charid, 0, 8).$hyphen
        .substr($charid, 8, 4).$hyphen
        .substr($charid,12, 4).$hyphen
        .substr($charid,16, 4).$hyphen
        .substr($charid,20,12);
        return $uuid;
      }
    }

/**
 * 根據vocabulary機器碼取得機關別分類Publisher IP相關資訊.
 * @return Array
 */
function ndcop_api_get_taxonomy_term_ip_info($vocabulary_name){

  $publisher_ip_set = array();
  $orgs = ndcop_api_get_taxonomy_term_names('org');

  foreach($orgs as $tid){
    $org_term = taxonomy_term_load($tid);
    if( !empty( $org_term->field_publisher_ip[LANGUAGE_NONE] ) ){
      $publisher_ip_set[$org_term->tid] = array(
        'name'=>$org_term->name,
        'ip_set'=>$org_term->field_publisher_ip[LANGUAGE_NONE][0]['value'],
        'oid'=>$org_term->field_orgoid[LANGUAGE_NONE][0]['value'],
        );
    }
  }

  return $publisher_ip_set;
}

/**
 * 根據APIIN Data中的publisherOID取得NDC平臺上的機關別分類相關資訊.
 * @param $publisherOID: APIIN Data中的publisherOID(機關OID)
 * @param $publisherName: APIIN Data中的publisher(機關名稱)
 * @param $publisherOrgCode: APIIN Data中的publisherOrgCode(機關代碼)
 * @return tid OR -1:在NDC平臺上找不到該publisherOID的機關別 OR -2:在NDC平臺上找到兩個以上符合該publisherOID的機關別
 */
function ndcop_api_get_ndc_org($publisherOID, $publisherName, $publisherOrgCode){

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'taxonomy_term')
  ->entityCondition('bundle', array('org'))
  ->fieldCondition('field_orgoid', 'value', $publisherOID);
  $result = $query->execute();

  if( !is_null($result['taxonomy_term']) ){
    $tid = array_keys($result['taxonomy_term']);
    if( count($tid) == 1 ){
      return $tid[0];
    }
    else{
      // 在NDC平臺上找到兩個以上符合該publisherOID的機關別
      return -2;
    }
  }
  else{
    $org_info = array(
      'oid' => $publisherOID,
      'orgname' => $publisherName,
      'orgcode' => $publisherOrgCode,
      );
    try{
      // 利用APIIN Data中的資訊建立
      $tid = ndcop_api_create_org_taxonomy_term( $org_info );
      if( !empty($tid) ){
        return $tid;
      }
      else{
        // 利用orgoid table的資訊建立
        $universal_oid_set_check = ndcop_api_check_universal_oid_set($publisherOID);

        if( $universal_oid_set_check['pass'] ){
          // 新增機關別分類
          $tid = ndcop_api_create_org_taxonomy_term( $universal_oid_set_check['values'] );
          return ( !empty($tid) ) ? $tid : -1;
        }
        else{
          return -1;
        }
      }
    }catch(Exception $e){
      watchdog('ndcop_api','新增機關別分類發生異常，機關名稱:%orgname，機關代碼:%orgcode，OID:%oid',array('%orgname' => $org_info['orgname'], '%orgcode' => $org_info['orgcode'], '%oid' => $org_info['oid'], WATCHDOG_DEBUG));
      return null;
    }

  }
}

/**
 *  判斷APIIN Data publisherOID 是否存在於另外建立的orgoid table
 *  @param $publisherOID: APIIN Data中的publisherOID
 *  @return (Array)pass: 是否通過檢查 ; values: 資料庫回傳結果
 */
function ndcop_api_check_universal_oid_set($publisherOID){
  // $back_login_oid = '2.16.886.101.20003.20069';
  $query = db_select('orgoid','orgoid');
  $query->addField('orgoid','orgname','orgname');
  $query->addField('orgoid','orgcode','orgcode');
  $query->addField('orgoid','oid','oid');
  $query->condition('orgoid.oid',$publisherOID );
  $results = $query->execute()->fetchAssoc();
  return array('pass' => !empty($results), 'values' => $results );
}

/**
 *  新增機關別taxonomy(org)的term
 *  @see ndcop_api_get_ndc_org
 *  @param $org_info: (Array)
 *  @return $term->tid: 機關別分類tid
 */
function ndcop_api_create_org_taxonomy_term( $org_info ){

  $vacabulary_obj =  taxonomy_vocabulary_machine_name_load('org');

  $term = new stdClass();
  $term->name = trim($org_info['orgname']);
  $term->vid = $vacabulary_obj->vid;
  $term->field_org_num[LANGUAGE_NONE][0]['value'] = trim($org_info['orgcode']);
  $term->field_orgoid[LANGUAGE_NONE][0]['value'] = trim($org_info['oid']);
  $term->field_serial_num[LANGUAGE_NONE][0]['value'] = '1';
  taxonomy_term_save($term);
  watchdog('ndcop_api','新增機關別分類，機關名稱:%orgname，機關代碼:%orgcode，OID:%oid',
   array('%orgname' => $org_info['orgname'], '%orgcode' => $org_info['orgcode'], '%oid' => $org_info['oid'], WATCHDOG_DEBUG));

  return $term->tid;
}

/**
 *  姓名遮蔽
 *  @see
 *  @param $nickname: String
 *  @return $masked_name: String
 */
function ndcop_api_mask_name( $nickname ){

  $masked_name = '';
  if(!empty($nickname)){
    $first_char = mb_substr($nickname, 0, 1,'utf-8');
    $last_char = mb_substr($nickname, -1, 1,'utf-8');
    $masked_name = $first_char.'Ｏ'.$last_char;
  }
  return $masked_name;
}
