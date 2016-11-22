<?php

/**
 * @file
 * 負責處理資料項目(resources).
 * TODO:
 */

/**
 *  新增資料集時新增Resources
 *  @see ndcop_dataset_api_create_dataset
 *  @param $dataset: Host Entity
 *  @param $distributions: NDCOPDATA規範的 API形式的Distributions Array
 */
function ndcop_dataset_api_create_resources( $dataset, $distributions ){
	watchdog('ndcop_dataset_api','新增資料集時新增Resources, distributions=%dis', array('%dis' => print_r($distributions, true), WATCHDOG_DEBUG));

	// 針對每一個資料資源
	foreach ($distributions as $key => $distribution) {
	    // create a blank FieldCollectionItemEntity instance
		$field_collection_item = entity_create('field_collection_item', array('field_name' => NDCOP_API_RESOURCE));

	    // Bind those two via this function
		$field_collection_item->setHostEntity('node', $dataset);

	    // We will use entitymetadatawrapper to save fields
		$resource = entity_metadata_wrapper('field_collection_item', $field_collection_item);

	    // 資料項目編號 TODO
		$resource->{NDCOP_API_RESOURCE_ID}->set($distribution->resourceID);
	    // 資料項目描述
		if( isset($distribution->resourceDescription) and !empty($distribution->resourceDescription) ){
			$resource->{NDCOP_API_RESOURCE_DESCRIPTION}->set( $distribution->resourceDescription );
		}

	    // 檔案格式
		if( isset($distribution->format) and !empty($distribution->format) ){
			$format_tid_mapping_arr = ndcop_api_get_taxonomy_term_names('format_gov');
		    // 檢查檔案格式是否已存在於分類
			$format_tid = $format_tid_mapping_arr[ strtoupper($distribution->format) ];

		    // 新增檔案格式分類
			if(empty($format_tid)){
				$vocabulary_obj =  taxonomy_vocabulary_machine_name_load('format_gov');
				$term = new stdClass();
				$term->name = strtoupper($distribution->format);
				$term->vid = $vocabulary_obj->vid;
				taxonomy_term_save($term);
				$format_tid = $term->tid;
			}
			$resource->{NDCOP_API_RESOURCE_FORMAT}->set( $format_tid );
		}

	    // 編碼格式
		if( isset($distribution->characterSetCode) and !empty($distribution->characterSetCode) ){
			$resource->{NDCOP_API_RESOURCE_CHARACTER_SET_CODE}->set( $distribution->characterSetCode );
		}

	    // 資料項目最後更新時間
		if( isset($distribution->resourceModified) and !empty($distribution->resourceModified) ){
			$resource->{NDCOP_API_RESOURCE_MODIFIED}->set( strtotime($distribution->resourceModified) );
		}

		if( isset($distribution->downloadURL) and !empty($distribution->downloadURL) ){
	      // 資料項目網址型態
			$resource->{NDCOP_API_RESOURCE_URLTYPE}->set( 'download' );
	      //此種寫法才能放入有中文的網址(上面的寫法會被link field validate擋住造成錯誤)
			$field_collection_item->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][] = array('url'=>$distribution->downloadURL);
		}
		elseif( isset($distribution->accessURL) and !empty($distribution->accessURL) ){
	      // 資料項目網址型態
			$resource->{NDCOP_API_RESOURCE_URLTYPE}->set( 'access' );
	      //此種寫法才能放入有中文的網址(上面的寫法會被link field validate擋住造成錯誤)
			$field_collection_item->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][] = array('url'=>$distribution->accessURL);
		}

	    // 領域別詮釋資料參考網址
		if( isset($distribution->metadataSourceOfData) and !empty($distribution->metadataSourceOfData) ){
	    	//此種寫法才能放入有中文的網址(上面的寫法會被link field validate擋住造成錯誤)
			$field_collection_item->{NDCOP_API_RESOURCE_METADATA_SOURCE}[LANGUAGE_NONE][] = array('url'=>$distribution->metadataSourceOfData);
		}

	    // // 資料項目對應政府詮釋資料集NID
		$resource->{NDCOP_API_RESOURCE_NID_REF}->set( $dataset->nid );
	    // APIIN的資料項目編號
		if( isset($distribution->resourceID) and !empty($distribution->resourceID) ){
			$resource->{NDCOP_API_RESOURCE_APIIN_ID}->set( $distribution->resourceID );
		}
	    // Finally save our item
		$field_collection_item->save();

	}
}

/**
 *  更新Resource
 *  @see ndcop_dataset_api_modify_dataset_resources
 *  @param $dataset: Host Entity
 *  @param $distribution: NDCOPDATA規範的 API形式的Distribution Array
 *  @param $update_rid: 欲更新之resourceID
 */

function ndcop_dataset_api_update_resource($dataset, $distribution, $update_rid ){

	watchdog('ndcop_dataset_api','更新Resource, distributions=%dis', array('%dis' => print_r($distribution, true), WATCHDOG_DEBUG));

	foreach( $dataset->{NDCOP_API_RESOURCE}[LANGUAGE_NONE] as $fc){

		$field_collection_item_id = $fc['value'];
		$field_collection_item = field_collection_item_load($field_collection_item_id);
	  // 比對欲修改的resource是否為此詮釋資料$dataset中的其中一筆資料項目
		if( $field_collection_item->{NDCOP_API_RESOURCE_APIIN_ID}[LANGUAGE_NONE][0]['safe_value'] == $update_rid ){

	  	 // We will use entitymetadatawrapper to save fields
			$resource = entity_metadata_wrapper('field_collection_item', $field_collection_item);

	    // 資料項目描述
			if( isset($distribution->resourceDescription) and !empty($distribution->resourceDescription) ){
				$resource->{NDCOP_API_RESOURCE_DESCRIPTION}->set( $distribution->resourceDescription );
			}

	    // 檔案格式
			if( isset($distribution->format) and !empty($distribution->format) ){
				$format_tid_mapping_arr = ndcop_api_get_taxonomy_term_names('format_gov');
		    // 檢查檔案格式是否已存在於分類
				$format_tid = $format_tid_mapping_arr[ strtoupper($distribution->format) ];

		    // 新增檔案格式分類
				if(empty($format_tid)){
					$vocabulary_obj =  taxonomy_vocabulary_machine_name_load('format_gov');
					$term = new stdClass();
					$term->name = strtoupper($distribution->format);
					$term->vid = $vocabulary_obj->vid;
					taxonomy_term_save($term);
					$format_tid = $term->tid;
				}
				$resource->{NDCOP_API_RESOURCE_FORMAT}->set( $format_tid );
			}

	    // 編碼格式
			if( isset($distribution->characterSetCode) and !empty($distribution->characterSetCode) ){
				$resource->{NDCOP_API_RESOURCE_CHARACTER_SET_CODE}->set( $distribution->characterSetCode );
			}

	    // 資料項目最後更新時間
			if( isset($distribution->resourceModified) and !empty($distribution->resourceModified) ){
				$resource->{NDCOP_API_RESOURCE_MODIFIED}->set( strtotime($distribution->resourceModified) );
			}

			if( isset($distribution->downloadURL) and !empty($distribution->downloadURL) ){
	      // 資料項目網址型態
				$resource->{NDCOP_API_RESOURCE_URLTYPE}->set( 'download' );
	      // 資料項目網址
	      // $resource->{NDCOP_API_RESOURCE_URL}->url = $distribution->downloadURL ;
	      //此種寫法才能放入有中文的網址(上面的寫法會被link field validate擋住造成錯誤)
				$field_collection_item->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][0] = array('url'=>$distribution->downloadURL);
			}
			elseif( isset($distribution->accessURL) and !empty($distribution->accessURL) ){
	      // 資料項目網址型態
				$resource->{NDCOP_API_RESOURCE_URLTYPE}->set( 'access' );
	      // 資料項目網址
	      // $resource->{NDCOP_API_RESOURCE_URL}->url = $distribution->accessURL ;
	      //此種寫法才能放入有中文的網址(上面的寫法會被link field validate擋住造成錯誤)
				$field_collection_item->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][0] = array('url'=>$distribution->accessURL);
			}

	    // 領域別詮釋資料參考網址
			if( isset($distribution->metadataSourceOfData) and !empty($distribution->metadataSourceOfData) ){
	    	//此種寫法才能放入有中文的網址(上面的寫法會被link field validate擋住造成錯誤)
				$field_collection_item->{NDCOP_API_RESOURCE_METADATA_SOURCE}[LANGUAGE_NONE][] = array('url'=>$distribution->metadataSourceOfData);
			}

	     // Finally save our item
		// save要帶參數TRUE，才不會影響原本的host
			$field_collection_item->save(TRUE);

		}
	}
}

/**
 *  刪除Resource
 *  @see ndcop_dataset_api_modify_dataset_resources
 *  @param $dataset: Host Entity
 *  @param $update_rid: 欲更新之resourceID
 */
function ndcop_dataset_api_delete_resource( $dataset, $delete_rid ){

	// echo $rid;print_r($dataset);exit();

	foreach( $dataset->{NDCOP_API_RESOURCE}[LANGUAGE_NONE] as $fc){

		$field_collection_item_id = $fc['value'];
		$field_collection_item = field_collection_item_load($field_collection_item_id);

	   // 比對欲刪除的resource是否為此詮釋資料$dataset中的其中一筆資料項目
		if( $field_collection_item->{NDCOP_API_RESOURCE_APIIN_ID}[LANGUAGE_NONE][0]['safe_value'] == $delete_rid && !is_null($delete_rid)){
			$field_collection_item->deleteRevision();
		}
	}

}

 /**
 *  同步詮釋資料所屬的資源供應資訊(Resources)(Distribution)
 *  @see ndcop_dataset_api_update_dataset
 *  @param $update_dataset: Host Entity
 *  @param $ndc_opdata_format_obj(Array): NDCOPDATA規範的API形式的json data object
 */
 function ndcop_dataset_api_modify_dataset_resources($update_dataset, $ndc_opdata_format_obj){
	// 新增、修改、刪除的resources id
 	$modify_dataset_rid = array();

	// 欲新增resources
 	$create_distributions = array();

 	$modify_dataset_rid = ndcop_dataset_api_get_modify_dataset_rid($update_dataset, $ndc_opdata_format_obj);

	// 於更新的詮釋資料內新增resource
 	foreach ($modify_dataset_rid['create'] as $rid) {
 		$distribution = array_filter($ndc_opdata_format_obj->distribution, function($x) use($rid) {
 			return ( $x->resourceID == $rid );
 		});
		// Rest key form 0
 		$distribution = array_values($distribution);
 		$create_distributions[] = $distribution[0];
 	}
 	if( count($create_distributions) > 0 ){
		// 一次新增多筆
 		ndcop_dataset_api_create_resources( $update_dataset, $create_distributions );
 	}

	// 於更新的詮釋資料內修改resource
 	foreach ($modify_dataset_rid['update'] as $rid) {
 		$distribution = array_filter($ndc_opdata_format_obj->distribution, function($x) use($rid) {
 			return ( $x->resourceID == $rid );
 		});
 		$distribution = array_values($distribution);

 		ndcop_dataset_api_update_resource( $update_dataset, $distribution[0], $rid );
 	}

	// 於更新的詮釋資料內刪除resource
 	foreach ($modify_dataset_rid['delete'] as $distribution_no => $rid) {
 		ndcop_dataset_api_delete_resource( $update_dataset, $rid );
 	}
 }

 /**
 *  取得需新增、修改或刪除的詮釋資料所屬的資源供應資訊(Resources)(Distribution)的rid
 *  @see ndcop_dataset_api_modify_dataset_resources
 *  @param $update_dataset: Host Entity
 *  @param $ndc_opdata_format_obj(Array): NDCOPDATA規範的API形式的json data object
 *  @return $modify_dataset_rid(Array): create Array, update Array, delete Array
 */
 function ndcop_dataset_api_get_modify_dataset_rid($update_dataset, $ndc_opdata_format_obj){
	// 新增、修改、刪除的resources id
 	$modify_dataset_rid = array();

	// 取得政府資料開放平臺上資料集相對應之Resources 的apiin id清單
 	$ndcop_apiin_rid_list = array();
	// 取得apiin JSONDATA資料集相對應之Resources的id清單
 	$apiin_rid_list = array();

 	$ndcop_apiin_rid_list =
 	ndcop_dataset_api_get_ndcop_dataset_apiin_resourceid_list($update_dataset);

 	$apiin_rid_list = ndcop_dataset_api_get_apiin_dataset_resourceid_list($ndc_opdata_format_obj->distribution);

	// 將這兩份清單UNION
 	$union_rid_ist = array_values( array_unique( array_merge( $ndcop_apiin_rid_list, $apiin_rid_list ) ) );
 	$exist_ndcop_apiin_rid_ist = FALSE;
 	$exist_apiin_rid_ist = FALSE;
 	foreach ($union_rid_ist as $key => $check_rid) {
		// A: 出現在政府資料開放平臺上資料集相對應之Resources 的apiin id清單
 		if( in_array($check_rid, $ndcop_apiin_rid_list) ){
 			$exist_ndcop_apiin_rid_ist = TRUE;
 		}
		// B: 出現在apiin JSONDATA資料集相對應之Resources的id清單
 		if( in_array($check_rid, $apiin_rid_list) ){
 			$exist_apiin_rid_ist = TRUE;
 		}

		// 須新增資料項目
 		if( !$exist_ndcop_apiin_rid_ist and $exist_apiin_rid_ist ){
 			$modify_dataset_rid['create'][] = $check_rid;
 		}
		// 須更新原有資料項目
 		elseif( $exist_ndcop_apiin_rid_ist and $exist_apiin_rid_ist ){
 			$modify_dataset_rid['update'][] = $check_rid;
 		}
		// 須刪除原有資料項目
 		elseif( $exist_ndcop_apiin_rid_ist and !$exist_apiin_rid_ist ){
 			$modify_dataset_rid['delete'][] = $check_rid;
 		}

		// Reset
 		$exist_ndcop_apiin_rid_ist = FALSE;
 		$exist_apiin_rid_ist = FALSE;
 	}

 	return $modify_dataset_rid;
 }

 /**
 *  取得政府資料開放平臺上資料集相對應之Resources 的apiin id清單
 *  @see
 *  @param $dataset: Host Entity
 *  @return $resource_id_list(Array)
 */
 function ndcop_dataset_api_get_ndcop_dataset_apiin_resourceid_list($dataset){
 	$resource_id_list = array();
 	foreach( $dataset->{NDCOP_API_RESOURCE}[LANGUAGE_NONE] as $fc){
 		$field_collection_item_id = $fc['value'];
 		$resource = field_collection_item_load($field_collection_item_id);
	  // 取得 API IN的resource id
 		$resource_id_list[] = $resource->{NDCOP_API_RESOURCE_APIIN_ID}[LANGUAGE_NONE][0]['safe_value'];
 	}

 	return $resource_id_list;

 }

/**
 *  取得apiin JSONDATA資料集相對應之Resources的id清單
 *  @see
 *  @param $distributions: API IN JSONDATA distributions
 *  @return $resource_id_list(Array)
 */
function ndcop_dataset_api_get_apiin_dataset_resourceid_list($distributions){

	$resource_id_list = array();

	foreach ($distributions as $distribution) {
		$resource_id_list[] = $distribution->resourceID;
	}

	return $resource_id_list;
}

 /**
 *  利用Resource ID取得JSON DATA所屬的資源供應資訊(Resources)(Distribution)
 *  @see
 *  @param $update_dataset: Host Entity
 *  @param $ndc_opdata_format_obj(Array): NDCOPDATA規範的API形式的json data object
 *  @return $modify_dataset_rid(Array):
 */
 function ndcop_dataset_api_get_distribution_by_rid($distributions, $rid){

 	$get_distribution = array();
 	foreach ($distributions as $key => $distribution) {
 		if(strcmp($distribution->resourceID, $rid) == 0){
 			$get_distribution[] = $distribution;
 		}
 	}
 	return $get_distribution;
 }

/**
 *  利用field collection ID取得API IN的資料資源序號(field_resource_in_id_g)
 *  @see
 *  @param $field_collection_item_id: field collection ID
 *  @return $resource_in_id_g: API IN的資料資源序號(field_resource_in_id_g)
 */
function ndcop_dataset_api_get_resource_in_id_g( $field_collection_item_id ){

	$query = db_select('field_data_'.NDCOP_API_RESOURCE_APIIN_ID, NDCOP_API_RESOURCE_APIIN_ID);
	$query->condition(NDCOP_API_RESOURCE_APIIN_ID.'.entity_id', $field_collection_item_id);
	$query->addField(NDCOP_API_RESOURCE_APIIN_ID,NDCOP_API_RESOURCE_APIIN_ID.'_value','api_in_identifier');
	$resource_in_id_g = $query->execute()->fetchField();
	return $resource_in_id_g;
}
