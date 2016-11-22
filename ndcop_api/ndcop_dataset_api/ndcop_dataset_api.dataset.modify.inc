<?php

/**
 * @file
 * 負責處理資料集詮釋資料API之資料集異動(新增、修改、刪除).
 * TODO:
 */

/**
 *  新增詮釋資料
 *  @see ndcop_dataset_api_control_dataset, ndcop_api_check_apikey_info
 *  @param $data: NDCOPDATA規範的 API形式的json data string
 *  @param $apikey_info: 根據Request Header的APIKey所帶的publisher資訊
 */
function ndcop_dataset_api_create_dataset($data, $apikey_info){
	$ndc_opdata_format_obj = json_decode( $data );
	$dataset = new stdClass();
  $dataset->title = $ndc_opdata_format_obj->title;
  $dataset->type = "metadataset";
  node_object_prepare($dataset); // Sets some defaults. Invokes hook_prepare() and hook_node_prepare()
  $dataset->uid = 1;
  $dataset->status = 1;
  $dataset->language = LANGUAGE_NONE;
  $dataset->comment = 2;

  // 資料集審查狀態直接設定為審查通過
  $dataset->field_dataset_status[LANGUAGE_NONE][0]['value'] = '4';

  $dataset = ndcop_dataset_api_modify_dataset_node('create', $dataset, $ndc_opdata_format_obj, $apikey_info);
  $dataset = node_submit($dataset); // Prepare node for a submit

  node_save($dataset); // After this call we'll get a nid


  watchdog('ndcop_dataset_api','欲新增資料資源: %jdo',
   array('%jdo' => print_r($ndc_opdata_format_obj->distribution, true), WATCHDOG_DEBUG));
  // 資源供應資訊 Distribution
  if( !empty($ndc_opdata_format_obj->distribution) ){
   ndcop_dataset_api_create_resources($dataset, $ndc_opdata_format_obj->distribution );
 }
  //意見回覆comment
 if(!empty($ndc_opdata_format_obj->Comments)){
  ndcop_dataset_api_create_comment($dataset, $ndc_opdata_format_obj->Comments);
}

watchdog('ndcop_dataset_api','開始新增結束, NDCOP identifier: %jdo',
 array('%jdo' => print_r($dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['value'], true), WATCHDOG_DEBUG));
return $dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['value'];
}

/**
 *  修改詮釋資料
 *  @see ndcop_dataset_api_control_dataset_identifier, ndcop_api_check_apikey_info
 *  @param $data: NDCOPDATA規範的 API形式的json data string
 *  @param $apikey_info: 根據Request Header的APIKey所帶的publisher資訊
 *  @param $update_dataset_nid: 欲修改的詮釋資料NID
 */
function ndcop_dataset_api_update_dataset($data, $apikey_info, $update_dataset_nid){

  $ndc_opdata_format_obj = json_decode( $data );
  // LOG 開始新增
  watchdog('ndcop_dataset_api','開始修改, NID: %nid,json data obj: %jdo',
   array('%jdo' => print_r($ndc_opdata_format_obj, true), '%nid'=>$update_dataset_nid ,WATCHDOG_DEBUG));
  // 修改詮釋資料本身
  $dataset = node_load($update_dataset_nid, null, true);
  $dataset = ndcop_dataset_api_modify_dataset_node('update',$dataset, $ndc_opdata_format_obj, $apikey_info);

  // 利用Update API來達成暫時下架的議題
  // 因為Update API本身就隱含上架露出的概念
  // 所以不管該資料集之前是甚麼狀態，只要呼叫Update API就一定會露出於NDC平臺
  $dataset->status = 1;
  $dataset->field_dataset_status[LANGUAGE_NONE][0]['value'] = '4';
  $dataset->field_data_last_update_time[LANGUAGE_NONE][0]['value'] = time();

  node_save($dataset);

  // 更新詮釋資料所屬的資源供應資訊(Resources)(Distribution)
  ndcop_dataset_api_modify_dataset_resources($dataset, $ndc_opdata_format_obj);

  // 更新詮釋資料所屬的意見回覆(Resources)(Distribution)
  ndcop_dataset_api_create_comment($dataset, $ndc_opdata_format_obj->Comments,true);

  watchdog('ndcop_dataset_api','開始修改結束, NDCOP identifier: %jdo',
   array('%jdo' => print_r($dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['value'], true), WATCHDOG_DEBUG));
  return $dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['value'];
}

/**
 *  更新詮釋資料Node Object
 *  @see ndcop_dataset_api_update_dataset,
 *  @param $action: create OR update
 *  @param $dataset: Node Object
 *  @param $ndc_opdata_format_obj: API IN Json Object
 *  @param $apikey_info: 根據Request Header的APIKey所帶的publisher資訊
 *  @return $dataset: Updated Node Object
 */
function ndcop_dataset_api_modify_dataset_node($action, $dataset, $ndc_opdata_format_obj, $apikey_info){

  $dataset->title = $ndc_opdata_format_obj->title;

  // 分類編號
  $dataset->{NDCOP_API_CATEGORYCODE}[LANGUAGE_NONE][0]['tid'] = ndcop_dataset_api_get_categorytid( $ndc_opdata_format_obj->categoryCode );

  if( $action == 'create' ){
    // NDCOP平台資料集編號 > 最後決議改由各平臺自行控制(所以填入對方API IN資料集編號)
    // TODO:
    $dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->identifier;;
    // 對方API IN資料集編號
    $dataset->{NDCOP_API_APIIN_IDENTIFIER}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->identifier;
  }
    $dataset->field_dataset_modify_method[LANGUAGE_NONE][0]['value'] = '1';
  // 資料集描述
  $dataset->{NDCOP_API_DESCRIPTION}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->description;

  // 主要欄位說明
  $dataset->{NDCOP_API_FIELD_DESCRIPTION}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->fieldDescription;

  // 資料集類型
  $type_name_mapping = ndcop_api_generate_type_name_mapping();
  // 全部轉為小寫
  $type_name_mapping = array_flip(array_map('strtolower', $type_name_mapping));

  $datatype_tid_mapping_arr = ndcop_api_get_taxonomy_term_names('datatype');

  // 超過一個值
  if ( is_array($ndc_opdata_format_obj->type) ) {
    foreach ($ndc_opdata_format_obj->type as $delta => $type) {
      $type_name = $type_name_mapping[ strtolower( $type ) ];
      $dataset->{NDCOP_API_TYPE}[LANGUAGE_NONE][$delta]['tid'] = $datatype_tid_mapping_arr[ $type_name ];
    }
  }
  else{
    $type_name = $type_name_mapping[ strtolower( $ndc_opdata_format_obj->type ) ];

    $dataset->{NDCOP_API_TYPE}[LANGUAGE_NONE][0]['tid'] = $datatype_tid_mapping_arr[ $type_name ];
  }

  // 授權方式
  $dataset->{NDCOP_API_LICENSE}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->license;

  // 授權說明網址
  $dataset->{NDCOP_API_LICENSE_URL}[LANGUAGE_NONE][0]['url'] = $ndc_opdata_format_obj->licenseURL;

  // 計費方式
  $field_item_info = field_info_field(NDCOP_API_COST);
  if(isset($field_item_info['settings']['allowed_values']) ){
    $cost_allowed_values = array_flip($field_item_info['settings']['allowed_values']);
    $dataset->{NDCOP_API_COST}[LANGUAGE_NONE][0]['value'] = $cost_allowed_values[$ndc_opdata_format_obj->cost];
  }

  if( $ndc_opdata_format_obj->cost == '免費' ){
    if( isset($dataset->{NDCOP_API_COST_URL}[LANGUAGE_NONE]) ){
      unset($dataset->{NDCOP_API_COST_URL}[LANGUAGE_NONE]);
    }
    if( isset($dataset->{NDCOP_API_COST_LAW}[LANGUAGE_NONE]) ){
      unset($dataset->{NDCOP_API_COST_LAW}[LANGUAGE_NONE]);
    }
  }
  else{
    // 計費說明網址 (選擇性填寫)
    ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->costURL, NDCOP_API_COST_URL, 'text');

    // 計費法令依據 (選擇性填寫)
    ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->costLaw, NDCOP_API_COST_LAW, 'text');
  }

  // 資料集所屬管理機關名稱
  $dataset->{NDCOP_API_ORGANIZATION}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->organization;
  $dataset->{NDCOP_API_ORGANIZATION_OID}[LANGUAGE_NONE][0]['value'] = ndcop_dataset_api_get_parent_org_oid($ndc_opdata_format_obj->publisherOID);

  // 資料集所屬管理機關聯絡人姓名 (加上稱謂?), 目前只處理一個人的情況
  $organizationContactName = explode(" ", $ndc_opdata_format_obj->organizationContactName);
  $dataset->{NDCOP_API_ORGANIZATION_CONTACTNAME}[LANGUAGE_NONE][0]['value'] = $organizationContactName[0];


  // 資料集所屬管理機關聯絡人電話
  $dataset->{NDCOP_API_ORGANIZATION_CONTACTPHONE}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->organizationContactPhone;

  // 資料集所屬管理機關聯絡人電子郵件
  $dataset->{NDCOP_API_ORGANIZATION_CONTACTEMAIL}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->organizationContactEmail;

  // 資料集提供機關名稱
  // 目前提供從API Key對應回平台的資訊
  $dataset->{NDCOP_API_PUBLISHER}[LANGUAGE_NONE][0]['tid'] = ndcop_api_get_ndc_org($ndc_opdata_format_obj->publisherOID);
  $dataset->{NDCOP_API_PUBLISHER_OID}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->publisherOID;

  // 資料集提供機關聯絡人 (加上稱謂), 目前只處理一個人的情況
  $publishercontactName = explode(" ", trim($ndc_opdata_format_obj->publisherContactName) );
  $dataset->{NDCOP_API_PUBLISHER_CONTACTNAME}[LANGUAGE_NONE][0]['value'] = $publishercontactName[0];


  $field_item_info = field_info_field(NDCOP_API_PUBLISHER_CONTACTNAME_GENDER);
  if( isset($publishercontactName[1]) and isset($field_item_info['settings']['allowed_values']) ){
    $gender = $publishercontactName[1];
    $gender_allowed_values = array_flip($field_item_info['settings']['allowed_values']);
    $dataset->{NDCOP_API_PUBLISHER_CONTACTNAME_GENDER}[LANGUAGE_NONE][0]['value'] = $gender_allowed_values[$gender];
  }

  // 資料集提供機關聯絡人電話
  $dataset->{NDCOP_API_PUBLISHER_CONTACTPHONE}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->publisherContactPhone;

  // 資料集提供機關聯絡人電子郵件
  $dataset->{NDCOP_API_PUBLISHER_CONTACTEMAIL}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->publisherContactEmail;

  // 更新頻率
  $dataset->{NDCOP_API_ACCRUALPERIODICITY}[LANGUAGE_NONE][0]['value'] = $ndc_opdata_format_obj->accrualPeriodicity;

  // 發布時間 (建議填寫)
  if( !empty($ndc_opdata_format_obj->issued) ){
    $dataset->{NDCOP_API_ISSUED}[LANGUAGE_NONE][0]['value'] = strtotime($ndc_opdata_format_obj->issued);
  }
  // 最後更新時間 (建議填寫)
  if( !empty($ndc_opdata_format_obj->modified) ){
    $dataset->{NDCOP_API_MODIFIED}[LANGUAGE_NONE][0]['value'] = strtotime($ndc_opdata_format_obj->modified);
  }
  // 標籤/關鍵字(舊文字欄位field_keyword處理方式) (建議填寫)
  $dataset->field_keyword[LANGUAGE_NONE][0]['value'] = implode(",", $ndc_opdata_format_obj->keyword);

  // 標籤/關鍵字(新term ref欄位FIELD_KEYWORD處理方式) (建議填寫)
  // API IN DATA有值
  if( isset($ndc_opdata_format_obj->keyword) and !empty($ndc_opdata_format_obj->keyword) ){
    $tmp_keyword_tids = array(); //用來填寫關鍵字欄位的暫存tids

    $tmp_keyword_tids = array(); // 用來填寫關鍵字欄位的暫存tids
    $keyword_tid_mapping_arr = ndcop_api_get_taxonomy_term_names('tags');
    foreach ($ndc_opdata_format_obj->keyword as $delta => $keyword) {
      // 檢查關鍵字是否已存在於分類
      $keyword_tid = $keyword_tid_mapping_arr[ $keyword ];

      // 新增關鍵字分類
      if(empty($keyword_tid)){
        $vocabulary_obj =  taxonomy_vocabulary_machine_name_load('tags');
        $term = new stdClass();
        $term->name = $keyword;
        $term->vid = $vocabulary_obj->vid;
        taxonomy_term_save($term);
        $tmp_keyword_tids[] = $term->tid;
      }
      else{
        $tmp_keyword_tids[] = $keyword_tid;
      }
    }
    if( isset($dataset->{FIELD_KEYWORD}[LANGUAGE_NONE]) ){
      unset($dataset->{FIELD_KEYWORD}[LANGUAGE_NONE]);
    }
    $tmp_keyword_tids = array_unique($tmp_keyword_tids); // 剔除重複關鍵字
    foreach ($tmp_keyword_tids as $delta => $keyword_tid) {
      $dataset->{FIELD_KEYWORD}[LANGUAGE_NONE][$delta]['tid'] = $keyword_tid;
    }

  }
  else{
    if( isset($dataset->{FIELD_KEYWORD}[LANGUAGE_NONE]) ){
      unset($dataset->{FIELD_KEYWORD}[LANGUAGE_NONE]);
    }
  }

  // 資料量 (選擇性填寫)
  $dataset->{NDCOP_API_NUMBER_OF_DATA}[LANGUAGE_NONE][0]['value'] = intval( $ndc_opdata_format_obj->numberOfData );
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->numberOfData, NDCOP_API_NUMBER_OF_DATA, 'int');

  // 備註 (選擇性填寫)
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->notes, NDCOP_API_NOTES, 'text');

  //瀏覽次數
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->viewCount, NDCOP_API_VIEW_COUNT, 'int');
  //下載次數
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->downloadCount, NDCOP_API_DOWNLOAD_COUNT, 'int');


  // 開始收錄日期 (建議填寫)
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->temporalCoverageFrom, NDCOP_API_TEMPORAL_COVERAGE_FROM, 'datetime');

  // 結束收錄日期 (建議填寫)
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->temporalCoverageTo, NDCOP_API_TEMPORAL_COVERAGE_TO, 'datetime');

  // 資料集空間範圍 (建議填寫)
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->spatial, NDCOP_API_SPATIAL, 'text');

  // 資料集語系 (建議填寫)
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->language, NDCOP_API_LANGUAGE, 'text');

  // 資料集相關網址 (建議填寫)
  ndcop_dataset_api_optional_field_process($dataset, $ndc_opdata_format_obj->landingPage, NDCOP_API_LANDING_PAGE, 'url');

  if( !empty($ndc_opdata_format_obj->distribution) ){
    $dataset->{NDCOP_API_RESOURCE_COUNT}[LANGUAGE_NONE][0]['value'] = count($ndc_opdata_format_obj->distribution);
  }
  return $dataset;
}

/**
 *  根據 category_code取得Drupal分類的tid
 *  @see ndcop_dataset_api_create_dataset
 *  @param $data: NDCOPDATA規範的 API形式的json data string
 */
function ndcop_dataset_api_get_categorytid( $category_code ){

	$tids = ndcop_dataset_api_get_category_tids(0,'');
	$category_codes = array();
	foreach ($tids as $key => $tid) {
		$category_term_obj = taxonomy_term_load($tid);
		$code_id= $category_term_obj->field_code_id[LANGUAGE_NONE][0]['safe_value'];
    $category_codes[$code_id] = $tid;
  }

  return $category_codes[ $category_code ];
}

/**
 *  取得資料集所屬管理機關的OID
 *  @see ndcop_dataset_api_create_dataset
 *  @param $org_oid: 機關或單位的OID
 */
function ndcop_dataset_api_get_parent_org_oid($org_oid){
  $parent_org_oid = '';
  $oid_length = strlen($org_oid);

  // DB Access: Get taxonomy_term By org
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'taxonomy_term')
  ->entityCondition('bundle', array('org'))
  ->fieldCondition('field_orgoid', 'value', $org_oid, '=');
  $result = $query->execute();
  $org_tid = array_pop( array_keys($result['taxonomy_term']) );

  if(!empty($org_tid)){

    $term_obj = taxonomy_term_load($org_tid);

    if($oid_length <= 24){

      // 1|機關, 2|處室單位
      // 機關
      if($term_obj->field_is_org[LANGUAGE_NONE][0]['value'] == '1'){
        // 本身機關代碼
        $parent_org_oid = $org_oid;
      }
      // 處室單位
      elseif($term_obj->field_is_org[LANGUAGE_NONE][0]['value'] == '2'){
        $parent_org_oid = substr($org_oid,0,18);
      }
      else{
        $error_msg = '尚未設定機關別分類的\'是否為機關欄位\'';
        throw new Exception($error_msg);
      }
    }
    else{
      $parent_org_oid = substr($org_oid,0,24);
    }
  }
  else{
    $error_msg = '於新增資料集自動帶入資料集所屬管理機關時，發現org_oid 尚未新增於本平臺機關別分類中';
    throw new Exception($error_msg);
  }
  return $parent_org_oid;
}

/**
 *  處理建議填寫欄位和選擇性填寫欄位
 *  @see ndcop_dataset_api_create_dataset
 *  @param $dataset: Node Object (Call by Reference)
 *  @param $api_in_data: API IN的資料
 *  @param $field_name: 欄位名稱
 *  @param $data_type: 資料類型(text, datatime, url)
 */
function ndcop_dataset_api_optional_field_process(&$dataset, $api_in_data, $field_name, $data_type){

  // 若API IN的有資料
  if( isset( $api_in_data ) and !empty($api_in_data) ){
    if( $data_type == 'text' ){
      $dataset->{$field_name}[LANGUAGE_NONE][0]['value'] = $api_in_data;
    }
    elseif( $data_type == 'datetime' ){
      $dataset->{$field_name}[LANGUAGE_NONE][0]['value'] = strtotime($api_in_data);
    }
    elseif( $data_type == 'url' ){
      $dataset->{$field_name}[LANGUAGE_NONE][0]['url'] = $api_in_data;
    }
    elseif( $data_type == 'int' ){
      $dataset->{$field_name}[LANGUAGE_NONE][0]['value'] = intval( $api_in_data );
    }
  }
  // 若API IN的無此資料
  else{

    if( isset($dataset->{$field_name}[LANGUAGE_NONE]) ){
      unset($dataset->{$field_name}[LANGUAGE_NONE]);
    }
  }
}
