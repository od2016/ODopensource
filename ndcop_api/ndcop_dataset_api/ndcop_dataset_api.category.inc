<?php

/**
 * @file
 * 負責處理資料集詮釋資料API之分類群組.
 * TODO: 
 */

/**
 * Returns a list of categoryCode.
 */
function ndcop_dataset_api_category_list(){

	$output = array(
	    'help' => t('[功能說明]: 取得分類群組清單.  [服務路徑]: {SRU}/rest/group  [輸入參數]: "limit": 輸入值int, 限制最多回傳的資料筆數。 "offset": 輸入值int, 指定從第幾筆後開始回傳。可與limit配合使用達到分頁目的。  [輸出內容]: 開放平臺上所有資料集之分類編號(categoryCode)。 [備註]: 1. 分類編號(categoryCode)為電子化政府入口網提供的分類選項之服務分類編號3碼18 類(服務分類第1層)。'),
	);
	  try {
	  	// 參數 default limit: 100, offset: 0
	    $limit = ( isset($_GET['limit']) && !empty($_GET['limit']) && ndcop_api_validate_postive_int($_GET['limit']) ) ? check_plain( $_GET['limit'] ) : 100;
	    $offset = (isset($_GET['offset']) && $_GET['offset'] && ndcop_api_validate_postive_int($_GET['offset'])) ? check_plain( $_GET['offset'] ) : 0;
	    
	    // Drupal Cache機制
	    $cache_id = 'ndcop_dataset_api:get:group:limit:'.$limit.':offset:'.$offset;
	    $category_codes_cache = cache_get($cache_id, 'cache');
	    $category_codes = $category_codes_cache->data;
	    if (empty($category_codes_cache)) {
	 	  	$tids = ndcop_dataset_api_get_category_tids($offset, $limit);
			$category_codes = array();
			foreach ($tids as $key => $tid) {
				$category_term_obj = taxonomy_term_load($tid);
				$category_codes[] = $category_term_obj->field_code_id[LANGUAGE_NONE][0]['safe_value'];
			}
	      cache_set($cache_id, $category_codes, 'cache', CACHE_PERMANENT);
	    }
	    $output['success'] = TRUE;
	    $output['result'] = $category_codes;

	  }
	  catch (EntityMetadataWrapperException $e) {
	    $output['success'] = FALSE;
	    $output['result'] = array();
	  }

	  drupal_json_output($output);
}

/**
 * Returns a list of identifier of related categoryCode.
 */
function ndcop_dataset_api_category_show($category_code){

	$output = array(
    	'help' => t("[功能說明]: 取得群組相關的資料集編號.  [服務路徑]: {SRU}/rest/group/{categoryCode}  [資源定義]: {categoryCode}為分類編號。 [輸入參數]: 無參數。  [輸出內容]: \"categoryCode\": 群組的分類編號。 \"display_name\": 用來顯示的分類群組名稱，可以使用中文。 \"description\": 描述分類群組詳細內容。 \"package_count\": 群組內資料集數量。 \"created\": 建立日期。 \"packages\": 群組內資料集編號清單。 [備註]: 1. 分類編號(categoryCode)為電子化政府入口網提供的分類選項之服務分類編號3碼18 類(服務分類第1層)。 2. 此資源路徑categoryCode相等於資料集詮釋資料之分類編號。 3.categoryCode可經由「取得分類群組清單」API項目來取得。 4. 若查無資料，則回傳如下所示： \"Not found\""),
	);

	if( ndcop_api_validate_is_catagoryCode($category_code) ){

		// Drupal Cache機制
	    $cache_id = 'ndcop_dataset_api:get:group:'. $category_code;
	    $identifiers_cache = cache_get($cache_id, 'cache');
	    $identifiers = $identifiers_cache->data;

	    if (empty($identifiers_cache)) {
			$query = new EntityFieldQuery();
			$query->entityCondition('entity_type', 'taxonomy_term')
		  	  ->entityCondition('bundle', array('data_catalog'))
		  	  ->fieldCondition('field_code_id', 'value', $category_code, '=');
			$result = $query->execute();
			$tid = array_pop( array_keys($result['taxonomy_term']) );
			$category_term_obj = taxonomy_term_load($tid);	
			// DB Access: Get dataset identifier By tid of category_code
			$node_query = new EntityFieldQuery();

			$node_query->entityCondition('entity_type', 'node')
			  ->entityCondition('bundle', 'metadataset')
			  ->propertyCondition('status', NODE_PUBLISHED)
			  ->fieldCondition('field_code_name', 'tid', $tid);

			$result = $node_query->execute();

			// Generate dataset identifiers
			if (isset($result['node'])) {

			  $identifiers = array();
			  $dataset_items_nids = array_keys($result['node']);
			  $dataset_items = entity_load('node', $dataset_items_nids);

			  foreach ($dataset_items as $key => $dataset) {
			  	
			  	if ( isset($dataset->field_serialno[LANGUAGE_NONE][0]['value']) ) {
			  		array_push($identifiers, $dataset->field_serialno[LANGUAGE_NONE][0]['value']);
			  	}
			  }
			}

			cache_set($cache_id, $identifiers, 'cache', CACHE_PERMANENT);
	    }
		if(count($identifiers)>0){

			$created = isset($category_term_obj->field_category_created[LANGUAGE_NONE][0]['value'])? date('Y-m-d H:i:s',$category_term_obj->field_category_created[LANGUAGE_NONE][0]['value']) : '' ;

			$result = array(
				'categoryCode' => $category_code,
				'display_name' => $category_term_obj->name,
				'description' => $category_code.'類別',
				'package_count' => count($identifiers),
				'created' => $created,
				'packages' => $identifiers
			);

			$output['success'] = TRUE;
	    	$output['result'] = $result;
		}
		else{
			$output['success'] = FALSE;
	    	$output['result'] = 'Not found';
		}
		

	}
	else{
		$output['success'] = FALSE;
      	$output['error'] = array(
	        'error_type' => t('categoryCode Not Found'),
	        'message' => t('categoryCode Not Found'),
	      );
	}

	drupal_json_output($output);
}
