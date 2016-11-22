<?php

/**
 * @file
 * 負責處理資料集詮釋資料API之資料集清單和明細.
 * TODO:
 */


/**
 * Returns a list of dataset.
 */

function ndcop_dataset_api_dataset_list(){
	$output = array(
    'help' => t('[功能說明]: 取得資料集編號清單.  [服務路徑]: {SRU}/rest/dataset [輸入參數]: "modified": 日期, 詮釋資料最後更新時間大於或等於輸入之查詢條件。若無輸入，則傳回所有資料集編號清單。日期格式為 yyyy-MM-dd HH:mm:ss。 "limit": 輸入值int, 限制最多回傳的資料筆數。 "offset": 輸入值int, 指定從第幾筆後開始回傳。可與limit配合使用達到分頁目的。  [輸出內容]: 開放平臺上所有資料集之資料集編號。 [備註]: 1. 若查無資料，則回傳如下所示： []'),
    );

	try {
	  	// 參數: modified, limit, offset
    $modified = ( isset($_GET['modified']) && !empty($_GET['modified']) ) ? urldecode( check_plain( $_GET['modified'] ) ) : '';
    $limit = ( isset($_GET['limit']) && !empty($_GET['limit']) && ndcop_api_validate_postive_int($_GET['limit']) ) ? check_plain( $_GET['limit'] ) : '';
    $offset = (isset($_GET['offset']) && $_GET['offset'] && ndcop_api_validate_postive_int($_GET['offset'])) ? check_plain( $_GET['offset'] ) : 0;
    $oid = (isset($_GET['oid']) && $_GET['oid'] ) ? check_plain( $_GET['oid'] ) : '';

    // Drupal Cache機制
    $cache_id = 'ndcop_dataset_api:get:dataset:modified:'. $modified .':limit:'.$limit.':offset:'.$offset . ':oid:' .$oid;
    $identifiers_cache = cache_get($cache_id, 'cache');
    $identifiers = $identifiers_cache->data;
    if (empty($identifiers_cache)) {
      // DB Access
      $results = ndcop_dataset_api_get_dataset($modified, $offset, $limit, $oid);
      foreach ($results as $result) {
        $identifiers[] = $result->serialno;
      }
      cache_set($cache_id, $identifiers, 'cache', CACHE_PERMANENT);
    }
		// TODO: 利用elysia_cron定時清cahce


	   // Generate Results
    if(count($identifiers)>0){
     $output['success'] = TRUE;
     $output['result'] = $identifiers;
   }
   else{
			// Not Found
     $output['success'] = FALSE;
     $output['result'] = array();
   }

 }
 catch (EntityMetadataWrapperException $e) {
   $output['success'] = FALSE;
   $output['result'] = array();
 }

 drupal_json_output($output);
}

/**
 * Returns fields of dataset.
 */
function ndcop_dataset_api_dataset_show($identifier){

	// Sanitize $tag
	$identifier = check_plain($identifier);

	$output = array(
   'help' => t('[功能說明]: 取得資料集詮釋資料.  [服務路徑]: {SRU}/rest/dataset/{identifier} [資源定義]: {identifier}為資料集編號。  [輸入參數]: 無參數 [輸出內容]: 資料開放平臺所提供的資料集詮釋資料之欄位和內容。詮釋資料之欄位可包含「資料集詮釋資料標準規範」所制定的詮釋資料欄位與機關自行定義的詮釋資料欄位。 [備註]: 1. 若查無資料，則回傳如下所示： "Not found"'),
   );

	// if exist, return tag dataset_nid; if not exist, return 0
	$dataset_nid = ndcop_api_validate_in_identifier_list($identifier);


	// 在系統上存在此資料集
	if( $dataset_nid > 0 ){

		$dataset = node_load($dataset_nid);
		$ndc_format_result = ndcop_dataset_api_ndcopdata_api_format( $dataset );
		// Generate Results
		$output['success'] = TRUE;
    $output['result'] = $ndc_format_result;
  }
  else{
    $output['success'] = FALSE;
    $output['error'] = array(
     'error_type' => t('Not Found'),
     'message' => t('Not Found'),
     );
  }

  drupal_json_output($output);
}

/**
 * Returns fields of dataset.
 */
function ndcop_dataset_api_dataset_sync($dataset_nid){

  $output = array();

  // 在系統上存在此資料集
  if( $dataset_nid > 0 ){

    $dataset = node_load($dataset_nid);
    $ndc_format_result = ndcop_dataset_api_ndcopdata_api_format( $dataset );
    $output = $ndc_format_result;
    return drupal_json_encode($output);
  }
  else{
    return false;
  }

}

/**
 *  將Drupal Node Array轉成NDCOPDATA規範的 API形式
 *  @see ndcop_dataset_api_dataset_show
 *  @param (Array)$dataset: drupal node array
 *  @return (Array)json_encode($ndcopdata_api_format): json string
 */
function ndcop_dataset_api_ndcopdata_api_format( $dataset ){
  // 分類編號
  $categoryCode = ndcop_dataset_api_check_isset($dataset->{NDCOP_API_CATEGORYCODE}[LANGUAGE_NONE][0]['tid']);
  if($categoryCode){
    $category_term_obj = taxonomy_term_load($categoryCode);
    $ndcopdata_api_format->categoryCode = $category_term_obj->field_code_id[LANGUAGE_NONE][0]['safe_value'];
  }else{
    $ndcopdata_api_format->categoryCode = '';
  }

  // 資料集編號
  $ndcopdata_api_format->identifier = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集名稱
  $ndcopdata_api_format->title = ndcop_dataset_api_check_isset( $dataset->title );

  // 資料集描述
  $ndcopdata_api_format->description = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_DESCRIPTION}[LANGUAGE_NONE][0]['safe_value'] );

  // 主要欄位說明
  $ndcopdata_api_format->fieldDescription = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_FIELD_DESCRIPTION}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集類型
  $type_name = array('原始資料'=>'rawData', '行動化應用軟體'=>'APPS', '其他'=>'TOOLS', '系統介接程式'=>'SERVICES');
  $types = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_TYPE}[LANGUAGE_NONE] );
  if($types){
    // 唯一值
    if( count($types) == 1 ){
      $type_term_obj = taxonomy_term_load($types[0]['tid']);
      $ndcopdata_api_format->type = $type_name[ $type_term_obj->name ];
    }
    // 多值
    else{
      $type_terms = array();
      foreach ($types as $type) {
        $type_term_obj = taxonomy_term_load($type['tid']);
        $type_terms[] = $type_name[ $type_term_obj->name ];
      }
      $ndcopdata_api_format->type = $type_terms;
    }
  }else{
    $ndcopdata_api_format->type = '';
  }

  // 授權方式
  $ndcopdata_api_format->license = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_LICENSE}[LANGUAGE_NONE][0]['safe_value'] );

  // 授權說明網址
  $ndcopdata_api_format->licenseURL = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_LICENSE_URL}[LANGUAGE_NONE][0]['url'] );

  // 計費方式
  $ndcopdata_api_format->cost = ndcop_dataset_api_field_value_mapping( NDCOP_API_COST, ndcop_dataset_api_check_isset($dataset->{NDCOP_API_COST}[LANGUAGE_NONE][0]['value'] ) );
  // echo ndcop_dataset_api_check_isset($dataset->field_charge[LANGUAGE_NONE][0]['value'] );

  // 計費說明網址
  $ndcopdata_api_format->costURL = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_COST_URL}[LANGUAGE_NONE][0]['safe_value'] );

  // 計費法令依據
  $ndcopdata_api_format->costLaw = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_COST_LAW}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集所屬管理機關名稱
  $ndcopdata_api_format->organization = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_ORGANIZATION}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集所屬管理機關聯絡人姓名 (加上稱謂?)
  $ndcopdata_api_format->organizationContactName = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_ORGANIZATION_CONTACTNAME}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集所屬管理機關聯絡人電話
  $ndcopdata_api_format->organizationContactPhone = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_ORGANIZATION_CONTACTPHONE}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集所屬管理機關聯絡人電子郵件
  $ndcopdata_api_format->organizationContactEmail = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_ORGANIZATION_CONTACTEMAIL}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集提供機關名稱
  $publisher = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_PUBLISHER}[LANGUAGE_NONE][0]['tid'] );
  $publisher_term_obj = taxonomy_term_load($publisher);
  if($publisher){
    $ndcopdata_api_format->publisher = $publisher_term_obj->name;
  }else{
    $ndcopdata_api_format->publisher = '';
  }

  // 資料集提供機關聯絡人 (加上稱謂)
  $ndcopdata_api_format->publisherContactName = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_PUBLISHER_CONTACTNAME}[LANGUAGE_NONE][0]['safe_value'] ).' '.ndcop_dataset_api_field_value_mapping( NDCOP_API_PUBLISHER_CONTACTNAME_GENDER, ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_PUBLISHER_CONTACTNAME_GENDER}[LANGUAGE_NONE][0]['value'] ) );

  // 資料集提供機關聯絡人電話
  $ndcopdata_api_format->publisherContactPhone = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_PUBLISHER_CONTACTPHONE}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集提供機關聯絡人電子郵件
  $ndcopdata_api_format->publisherContactEmail = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_PUBLISHER_CONTACTEMAIL}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集提供機關 OID
  $ndcopdata_api_format->publisherOID = ndcop_dataset_api_check_isset( $publisher_term_obj->field_orgoid[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集提供機關 機關代碼
  $ndcopdata_api_format->publisherOrgCode = ndcop_dataset_api_check_isset( $publisher_term_obj->field_org_num[LANGUAGE_NONE][0]['safe_value'] );

  // 更新頻率
  $ndcopdata_api_format->accrualPeriodicity = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_ACCRUALPERIODICITY}[LANGUAGE_NONE][0]['safe_value'] );

  // $ndcopdata_api_format->temporalCoverage = $extras['temporalCoverage'];
  // 開始收錄日期(新規範欄位)
  $temporalCoverageFrom = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_TEMPORAL_COVERAGE_FROM}[LANGUAGE_NONE][0]['value'] );
  if($temporalCoverageFrom){
    $ndcopdata_api_format->temporalCoverageFrom = date( 'Y-m-d H:i:s', $temporalCoverageFrom );
  }else{
    $ndcopdata_api_format->temporalCoverageFrom = '';
  }

  // 結束收錄日期(新規範欄位)
  $temporalCoverageTo = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_TEMPORAL_COVERAGE_TO}[LANGUAGE_NONE][0]['value'] );
  if($temporalCoverageTo){
    $ndcopdata_api_format->temporalCoverageTo = date( 'Y-m-d H:i:s', $temporalCoverageFrom );
  }else{
    $ndcopdata_api_format->temporalCoverageTo = '';
  }

  // 發布時間
  $issued = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_ISSUED}[LANGUAGE_NONE][0]['value'] );
  if($issued){
    $ndcopdata_api_format->issued = date( 'Y-m-d H:i:s', $issued );
  }else{
    $ndcopdata_api_format->issued = '';
  }

  // 最後更新時間
  $modified = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_MODIFIED}[LANGUAGE_NONE][0]['value'] );
  if($modified){
    $ndcopdata_api_format->modified = date( 'Y-m-d H:i:s', $modified );
  }else{
    $ndcopdata_api_format->modified = '';
  }

  // 資料集空間範圍
  $ndcopdata_api_format->spatial = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_SPATIAL}[LANGUAGE_NONE][0]['safe_value'] );
  // 資料集語系
  $ndcopdata_api_format->language = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_LANGUAGE}[LANGUAGE_NONE][0]['safe_value'] );

  // 資料集相關網址
  $ndcopdata_api_format->landingPage = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_LANDING_PAGE}[LANGUAGE_NONE][0]['url']);
  // 標籤/關鍵字
  $keywords = ndcop_dataset_api_check_isset( $dataset->{FIELD_KEYWORD}[LANGUAGE_NONE] );
  if($keywords){
    $ndcopdata_api_format->keyword = ndcop_dataset_api_split_term_ref_keywords($keywords);
  }else{
    $ndcopdata_api_format->keyword = array();
  }

  // 資料量
  $number_of_data = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_NUMBER_OF_DATA}[LANGUAGE_NONE][0]['value'] );
  if( !empty( $number_of_data ) ){
    $ndcopdata_api_format->numberOfData = intval(  $dataset->{NDCOP_API_NUMBER_OF_DATA}[LANGUAGE_NONE][0]['value'] );
  }
  else{
    $ndcopdata_api_format->numberOfData = '';
  }

  // 備註
  $ndcopdata_api_format->notes = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_NOTES}[LANGUAGE_NONE][0]['safe_value'] );
  //瀏覽次數
  $ndcopdata_api_format->viewCount = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_VIEW_COUNT}[LANGUAGE_NONE][0]['value'] );
  //下載次數
  $ndcopdata_api_format->downloadCount = ndcop_dataset_api_check_isset( $dataset->{NDCOP_API_DOWNLOAD_COUNT}[LANGUAGE_NONE][0]['value'] );

  // 資源供應資訊 Distribution
  $ndcopdata_api_format->distribution =ndcop_dataset_api_generate_distribution_new($dataset);
  $ndcopdata_api_format->Comments = ndcop_dataset_api_generate_comment($dataset);
  return $ndcopdata_api_format;
}

/**
 *  針對舊版28欄位的處理方式
 *  產生dataset對應之distribution(Resouses)
 *  @see ndcop_dataset_api_ndcopdata_api_format
 *  @param (Obj)$dataset: Node Object
 *  @return (Array)$distribution:
 */
function ndcop_dataset_api_generate_distribution($dataset){

  // 產生 format term name的array, key:tid;value:format name
  $type_normal = array_flip( ndcop_api_get_taxonomy_term_names('data_normal') );
  $type_program = array_flip( ndcop_api_get_taxonomy_term_names('data_program') );
  $type_apps = array_flip( ndcop_api_get_taxonomy_term_names('data_apps') );
  $type_others = array_flip( ndcop_api_get_taxonomy_term_names('data_other') );
  $data_format = $type_normal+$type_program+$type_apps+$type_others;

  $data_format_machine_name = array();


  // 產生和dataset欄位有關的machine name array, ex: field_csv, field_xml...
  // key: tid; value: format machine name
  foreach ($data_format as $tid => $format_name) {
          // array_push($data_format_machine_name, "field_".strtolower($format_name));
    $data_format_machine_name[$tid] = 'field_'.strtolower($format_name);
  }
  // 產生distributions
  $distributions = array();
  foreach ($data_format_machine_name as $tid => $format_machine_name) {

    $dataset_format_item = field_get_items('node', $dataset, $format_machine_name);
    if(isset($dataset_format_item[0]['url'])){
      $distribution = new StdClass();
      $distribution->format = $data_format[$tid];
      $distribution->downloadURL = $dataset_format_item[0]['url'];

      // 暫時先取用儲存在詮釋資料的編碼格式
      $characterSetCode = field_get_items('node', $dataset, 'field_dataencoding');
      $distribution->characterSetCode = ndcop_dataset_api_check_isset( $characterSetCode[0]['safe_value'] );

      array_push($distributions, $distribution);
    }
  }
  return $distributions;
}

/**
 *  針對新版規範38欄位的處理方式
 *  產生dataset對應之distribution(Resouses)
 *  @see ndcop_dataset_api_ndcopdata_api_format
 *  @param (Obj)$dataset_node: Node Object
 *  @return (Array)$distribution:
 */
function ndcop_dataset_api_generate_distribution_new($dataset_node){

  // 產生distributions
  $distributions = array();

  $items = field_get_items('node', $dataset_node, NDCOP_API_RESOURCE);

  if( is_array($items) ){
    foreach ($items as $item) {
      $resource = field_collection_field_get_entity($item);
      $distribution = new StdClass();
      // 資料項目編號
      $distribution->resourceID = ndcop_dataset_api_check_isset($resource->{NDCOP_API_RESOURCE_ID}[LANGUAGE_NONE][0]['value']);
      // 資料項目描述
      $distribution->resourceDescription = ndcop_dataset_api_check_isset($resource->{NDCOP_API_RESOURCE_DESCRIPTION}[LANGUAGE_NONE][0]['value']);
      // 檔案格式
      if( !empty($resource->{NDCOP_API_RESOURCE_FORMAT}[LANGUAGE_NONE][0]['tid']) ){
        $format_term = taxonomy_term_load( $resource->{NDCOP_API_RESOURCE_FORMAT}[LANGUAGE_NONE][0]['tid'] );
        $distribution->format = strtoupper($format_term->name);
      }
      // 資料項目最後更新時間
      $resource_modified = ndcop_dataset_api_check_isset( $resource->{NDCOP_API_RESOURCE_MODIFIED}[LANGUAGE_NONE][0]['value'] );
      if($resource_modified){
        $distribution->resourceModified = date( 'Y-m-d H:i:s', $resource_modified );
      }else{
        $distribution->resourceModified = '';
      }
      // 資料項目網址型態
      if( !empty( $resource->{NDCOP_API_RESOURCE_URLTYPE}[LANGUAGE_NONE][0]['value'] ) ){
        $url_type = $resource->{NDCOP_API_RESOURCE_URLTYPE}[LANGUAGE_NONE][0]['value'];
        switch ($url_type) {
          case 'download':
          $distribution->downloadURL = ndcop_dataset_api_check_isset( $resource->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][0]['url'] );
          break;
          case 'access':
          $distribution->accessURL = ndcop_dataset_api_check_isset( $resource->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][0]['url'] );
          break;
          default:
          watchdog('ndcop_dataset_api','%data',
           array('%data' => '資料集詮釋資料資料GET API 項目網址型態有問題'. __FUNCTION__, WATCHDOG_DEBUG));
          throw new Exception("Error Processing Request: ndcop_dataset_api");
          break;
        }

      }

      // 領域別詮釋資料參考網址
      $distribution->metadataSourceOfData = ndcop_dataset_api_check_isset( $resource->{NDCOP_API_RESOURCE_METADATA_SOURCE}[LANGUAGE_NONE][0]['value'] );

      // 編碼格式
      $distribution->characterSetCode = ndcop_dataset_api_check_isset( $resource->{NDCOP_API_RESOURCE_CHARACTER_SET_CODE}[LANGUAGE_NONE][0]['value'] );

      array_push($distributions, $distribution);
    }

    return $distributions;

  }
}

/**
 *  產生dataset對應之comment
 *  @see ndcop_dataset_api_ndcopdata_api_format
 *  @param (Obj)$dataset_node: Node Object
 *  @return (Array)$distribution:
 */
function ndcop_dataset_api_generate_comment($dataset_node){

  // 產生distributions
  $comments = array();
  $type = array(
    '1'=>'資料集內容亂碼',
    '2'=>'資料集下載連結有問題',
    '3'=>'建議增加資料集內容',
    '5'=>'肯定與鼓勵',
    '6'=>'建議提供其他檔案類型',
    '7'=>'其他',
    '8'=>'資料集內容有誤',
    );
  $items = comment_get_thread($dataset_node,'COMMENT_MODE_FLAT','99');
  $comments_array=(comment_load_multiple($items));
  foreach ($comments_array as $item) {
    $comment = new StdClass();
    $comment->commentAuthor = ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_AUTHOR});
    $comment->commentType = $type[ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_TYPE}[LANGUAGE_NONE][0]['value'])];
    $comment->commentSubject = ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_SUBJECT});
    $comment->comments=ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_BODY}[LANGUAGE_NONE][0]['value']);
    $comment->commentId=ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_CID}[LANGUAGE_NONE][0]['value']);
    $comment->commentReplyId=ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_PID}[LANGUAGE_NONE][0]['value']);
    $comment->commentHost=ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_HOST}[LANGUAGE_NONE][0]['value']);
    $commentTime = ndcop_dataset_api_check_isset($item->{NDCOP_API_COMMENT_TIME}[LANGUAGE_NONE][0]['value']);
    if($commentTime){
      $comment->commentTime =  date( 'Y-m-d H:i:s', $commentTime);
    }else{
      $comment->commentTime = '';
    }
    $comment->commentEmail=ndcop_dataset_api_check_isset($item->{'mail'});
    array_push($comments, $comment);
  }
  return $comments;

}

/**
 *  將代碼轉換成對應文字
 *  @see ndcop_dataset_api_ndcopdata_api_format
 *  @param (String)$id
 *  @return (String)
 */
function ndcop_dataset_api_field_value_mapping($field_name, $value){

  $field_item_info = field_info_field($field_name);

  // text field widget使用select list設定之 allowed values
  if(isset($value) and isset($field_item_info['settings']['allowed_values']) ){
    return $field_item_info['settings']['allowed_values'][$value];
  }
  else{
    return '';
  }
}

/**
 * 取得指定oid底下所有metadataset數量總和
 * @param  string $oid 部會oid
 * @return boolean 結果或false
 */
function ndcop_dataset_api_get_node_count_by_oid($oid)
{
  $metadataset_info = array();
  $output = array(
   'help' => t('[功能說明]: 取得指定部會與其所屬部會所有已上架之資料集數量總和.  [服務路徑]: {SRU}/rest/org/{orgOid}/dataset_count_statistic [資源定義]:{orgOid}為欲查詢之部會oid，詳情參考 http://oid.nat.gov.tw/OIDWeb/chmain.html [輸入參數]: 無參數 [輸出內容]: 該部會與其所屬部會所有資料集數量總和 [備註]: 1. 若查無資料，則回傳如下所示： "Not found"'),
   );
  if(!ndcop_dataset_api_check_oid_is_validate($oid)){
    $output['success'] = false;
    $output['error'] = array(
     'error_type' => t('Not Found'),
     'message' => t('Not Found'),
     );
  }else{
    $rst = db_query("SELECT COUNT(*) FROM {node} n, {field_data_field_oid} ffo
     WHERE n.nid = ffo.entity_id
     AND n.status = '1'
     AND n.type = 'metadataset'
     AND ffo.field_oid_value LIKE :oid", array(
      ':oid' => db_like($oid). '%'))
    ->fetchField();
    $output['success'] = true;
    $output['result'] = array(
      'orgOid' => $oid,
      'datasetCountStatistic' => $rst);
  }
  drupal_json_output($output);
}

/**
 * 取得指定oid底下metadataset的identifier清單
 * @param  string $oid 部會oid
 * @return boolean 結果或false
 */
function ndcop_dataset_api_get_dataset_identifier_by_oid($oid)
{
  $metadataset_info = array();
  $output = array(
     'help' => t('[功能說明]: 取得指定部會所有已上架之資料集identifier清單.  [服務路徑]: {SRU}/rest/org/{orgOid}/dataset_identifier_list [資源定義]:{orgOid}為欲查詢之部會oid，詳情參考 http://oid.nat.gov.tw/OIDWeb/chmain.html [輸入參數]: 無參數 [輸出內容]: 該部會與其所屬部會所有資料集數量總和 [備註]: 1. 若查無資料，則回傳如下所示： "Not found"'),
     );
  if(!ndcop_dataset_api_check_oid_is_validate($oid)){
    $output['success'] = false;
    $output['error'] = array(
       'error_type' => t('Not Found'),
       'message' => t('Not Found'),
       );
}else{
    $cacheId = 'ndcop_dataset_api:get:dataset_identifier_list:oid:'.$oid;
    $identifiersListCache = cache_get($cacheId, 'cache');
    $identifiersList = $identifiersListCache->data;
    if (empty($identifiersList)) {
        $rstRaw = db_query("SELECT ffsn.field_serialno_value
            FROM node n
            INNER JOIN field_data_field_serialno ffsn ON n.nid = ffsn.entity_id
            INNER JOIN field_data_field_oid ffo ON n.nid = ffo.entity_id
            WHERE n.nid = ffsn.entity_id
            AND n.type = 'metadataset' AND n.status = '1'
            AND ffo.field_oid_value = :oid", array(':oid' => $oid))->fetchAll(PDO::FETCH_ASSOC);
        foreach ($rstRaw as $v) {
            $identifiersList[] = $v['field_serialno_value'];
        }
        cache_set($cacheId, $identifiersList, 'cache', time()+86400);
    }
    $output['success'] = true;
    $output['result'] = array('datasetIdentifierList' => $identifiersList);
}
drupal_json_output($output);
}
function ndcop_dataset_api_check_oid_is_validate($oid)
{
  $rst = db_query("SELECT * FROM {field_data_field_orgoid} ffo
   WHERE ffo.field_orgoid_value = :oid", array(':oid' => $oid))
  ->fetchAssoc();

  return $rst;
}
