<?php

/**
 * @file
 * 負責處理資料集詮釋資料API之標籤.
 * TODO:
 */

/**
 * Returns a list of tag.
 */

function ndcop_dataset_api_tag_list(){

	$output = array(
		'help' => t('[功能說明]: 取得標籤清單.  [服務路徑]: {SRU}/rest/tag  [輸入參數]: "limit": 輸入值int, 限制最多回傳的資料筆數。 "offset": 輸入值int, 指定從第幾筆後開始回傳。可與limit配合使用達到分頁目的。  [輸出內容]: 開放平臺上所有資料集之標籤清單。 [備註]: 1. 標籤(tag)為各業務機關紀錄於資料集詮釋資料中的標籤/關鍵字(keyword)。'),
		);
	try {
	  	// 參數 default limit: 100, offset: 0
		$limit = ( isset($_GET['limit']) && !empty($_GET['limit']) && ndcop_api_validate_postive_int($_GET['limit']) ) ? check_plain( $_GET['limit'] ) : 100;
		$offset = (isset($_GET['offset']) && $_GET['offset'] && ndcop_api_validate_postive_int($_GET['offset'])) ? check_plain( $_GET['offset'] ) : 0;

	    // Drupal Cache機制
		$cache_id = 'dataset_tags:get:tag:limit:'.$limit.':offset:'.$offset;

		$tags_cache = cache_get($cache_id, 'cache');
		$tags = $tags_cache->data;

		if (empty($tags_cache)) {
		$tags = ndcop_dataset_api_get_dataset_tags($offset, $limit);
		cache_set($cache_id, $tags, 'cache', CACHE_PERMANENT);
		} // Drupal Cache End

		$output['success'] = TRUE;
		$output['result'] = $tags;

	}
	catch (EntityMetadataWrapperException $e) {
		$output['success'] = FALSE;
		$output['result'] = array();
	}

	drupal_json_output($output);
}

/**
 * Returns a list of identifier of related tags.
 */
function ndcop_dataset_api_tag_show($tag){

	// Sanitize $tag
	$tag = check_plain($tag);

	$output = array(
		'help' => t('[功能說明]: 取得標籤相關的資料集編號.  [服務路徑]: {SRU}/rest/tag/{tagName} [資源定義]: {tagName}為標籤。 [輸入參數]: "limit": 輸入值int, 限制最多回傳的資料筆數。 "offset": 輸入值int, 指定從第幾筆後開始回傳。可與limit配合使用達到分頁目的。  [輸出內容]: 開放平臺上所有具有該標籤的資料集之資料集編號。 [備註]: 1. 標籤(tag)為各業務機關紀錄於資料集詮釋資料中的標籤/關鍵字(keyword)。 2. tagName可經由「取得標籤清單」API項目來取得。'),
		);

    $tag_name = ndcop_api_validate_tag_in_dataset_tags($tag);
	if($tag_name){

		// 參數 default limit: 100, offset: 0
		$limit = ( isset($_GET['limit']) && !empty($_GET['limit']) && ndcop_api_validate_postive_int($_GET['limit']) ) ? check_plain( $_GET['limit'] ) : 100;
		$offset = (isset($_GET['offset']) && $_GET['offset'] && ndcop_api_validate_postive_int($_GET['offset'])) ? check_plain( $_GET['offset'] ) : 0;

		// Drupal Cache機制
        $cache_id = 'ndcop_dataset_api:get:dataset_tag:'. $tag . ':limit:'.$limit.':offset:'.$offset;
		$identifiers_cache = cache_get($cache_id, 'cache');
		$identifiers = $identifiers_cache->data;

		if (empty($identifiers_cache)) {
			// DB Access: Get dataset identifier By tid of tag
			$node_query = new EntityFieldQuery();

			$node_query->entityCondition('entity_type', 'node')
			->entityCondition('bundle', 'metadataset')
			->propertyCondition('status', NODE_PUBLISHED)
            ->fieldCondition('field_dataset_tags', 'tag', $tag_name);
			  if ($limit) {
			  	$node_query->range($offset, $limit);
			  }

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
	    } // Drupal Cache End

		// Generate Results
	    if(count($identifiers)>0){
	    	$output['success'] = TRUE;
	    	$output['result'] = $identifiers;
	    }
	    else{
			// Not Found
	    	$output['success'] = FALSE;
	    	$output['result'] = '';
	    }


	}
	else{
		$output['success'] = FALSE;
		$output['error'] = array(
			'error_type' => t('tag Not Found'),
			'message' => t('tag Not Found'),
			);
	}

	drupal_json_output($output);
}
