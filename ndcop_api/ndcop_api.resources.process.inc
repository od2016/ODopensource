<?php
/**
 * @file
 * 負責處理資料集資料項目(resources)一對一轉換為一對多.
 * TODO: 
 */
module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.resource');

function ndcop_api_resources_transfer_batch_form(){

	$form = array();

	$form['resources']['transfer'] = array (
	    '#type' => 'submit',
	    '#value' => t('resources轉為一對多關係'),
	    '#submit' => array('ndcop_api_resources_transfer_batch_form_submit'),
	);

	return $form;
}

function ndcop_api_resources_transfer_batch_form_submit($form, &$form_state){

	$num = 0;
	$process_nids = array();

	$node_query = new EntityFieldQuery();
	$node_query->entityCondition('entity_type', 'node')
		  ->entityCondition('bundle', 'metadataset');
	$result = $node_query->execute();

	if (isset($result['node'])) {
		$dataset_items_nids = array_keys($result['node']);

		// 資料格式
		$data_format_machine_name = ndcop_api_resources_transfer_old_data_format();
	  	
	  	foreach ($dataset_items_nids as $key => $dataset_items_nid) {
	  		$process_nids[ $dataset_items_nid ] =  $dataset_items_nid;
	  		$num++;
			if( ($num % 50) == 0 ){
		        $operations[] = array(
		          'ndcop_api_transfer_resources_batch_processing',
		          array($process_nids, $data_format_machine_name),
		        );
		        $process_nids = array();
	    	}

	  	}

		$operations[] = array(
		      'ndcop_api_transfer_resources_batch_processing',
		      array($process_nids, $data_format_machine_name),
		    );
	  	$batch = array(
	        'title' => t('轉換Resources中...'),
	        'operations' => $operations,
	        'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.resources.process.inc',
	        'finished' => 'ndcop_api_transfer_resources_batch_processing_finished',
	        'error_message' => t('The installation has encountered an error.'),
	        'progress_message' => t('處理 @current / @total 資料集'),
	      );
	     watchdog('ndcop api','%data', 
	         array('%data' => '開始轉換Resources!', WATCHDOG_DEBUG));
	     $name = 'transfer_resouces';
    	 timer_start($name);
	     batch_set($batch);
	}

}

function ndcop_api_transfer_resources_batch_processing($nids, $data_format_machine_name, &$context){

	if ( !isset($context['results']['created']) ) {
    	$context['results']['created'] = 0;
  	}

  	foreach ($nids as $nid) {
  		$dataset = node_load($nid);
  		$distributions = ndcop_api_transfer_resources_create_distribution($dataset, $data_format_machine_name);
  		ndcop_api_transfer_resources_create_resources( $dataset, $distributions );
  		$context['results']['created']++;
  	}

}

function ndcop_api_transfer_resources_batch_processing_finished($success, $results, $operations){

	if($success){
		$name = 'transfer_resouces';
		$message = t('轉換資料集 @count 筆', array('@count' => $results['created'])).'<br />';
		drupal_set_message($message);
		watchdog('ndcop api','%data, Use Time: %time', array('%data' => '資料集一對一轉換為一對多完成!','%time' => timer_read($name).'ms', WATCHDOG_DEBUG));
		
	}
	else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    drupal_set_message(
      t('An error occurred while processing @operation with arguments : @args',
        array(
          '@operation' => $error_operation[0],
          '@args' => print_r($error_operation[0], TRUE),
        )
      )
    );
  }
}

function ndcop_api_transfer_resources_create_distribution($dataset, $data_format_machine_name){

	  // 產生distributions
	  $distributions = array();
	  foreach ($data_format_machine_name as $tid => $format_machine_name) {

	    $dataset_format_item = field_get_items('node', $dataset, $format_machine_name);
	    if(isset($dataset_format_item[0]['url']) and !empty($dataset_format_item[0]['url']) ){
	      $distribution = new StdClass();
	      $format_name = explode('_', $format_machine_name);
	      $distribution->format = $format_name[1];
	      $distribution->downloadURL = $dataset_format_item[0]['url'];
	      $characterSetCode = field_get_items('node', $dataset, 'field_dataencoding');
	      $distribution->characterSetCode = ndcop_dataset_api_check_isset( $characterSetCode[0]['safe_value'] );

	      array_push($distributions, $distribution);
	    }
	  }
	  return $distributions;

}

/**
 *  新增資料集時新增Resources
 *  @see ndcop_api_transfer_resources_batch_processing
 *  @param $dataset: Host Entity
 *  @param $distributions: NDCOPDATA規範的 API形式的Distributions Array
 */
function ndcop_api_transfer_resources_create_resources( $dataset, $distributions ){


	watchdog('ndcop api','%data, NID: %nid; Distributions: %resouces', array('%data' => '新增資料項目資訊','%nid' => $dataset->nid, '%resouces'=>print_r($distributions,true), WATCHDOG_DEBUG));
  $dataset_resource_count = isset($dataset->{NDCOP_API_RESOURCE_COUNT}[LANGUAGE_NONE][0]['value'])? $dataset->{NDCOP_API_RESOURCE_COUNT}[LANGUAGE_NONE][0]['value'] : 1 ;
  $resource_tmp_count = 0;
  $dataset_identifier = $dataset->{NDCOP_API_NDCOP_IDENTIFIER}[LANGUAGE_NONE][0]['value'];

	// 針對每一個資料資源
	foreach ($distributions as $key => $distribution) {
	    // create a blank FieldCollectionItemEntity instance
	    $field_collection_item = entity_create('field_collection_item', array('field_name' => NDCOP_API_RESOURCE));

	    // Bind those two via this function
	    $field_collection_item->setHostEntity('node', $dataset);

	    // We will use entitymetadatawrapper to save fields
	    $resource = entity_metadata_wrapper('field_collection_item', $field_collection_item);

	    // 資料項目編號
	    $new_resource_count = (hexdec($dataset_resource_count) + $resource_tmp_count); //16to10後再加
	    $hex_new_resource_count = dechex($new_resource_count);
	    $resource_id = ndcop_dataset_api_generate_resource_id($dataset_identifier, $hex_new_resource_count, 3); //補0
	    $resource->{NDCOP_API_RESOURCE_ID}->set($resource_id);
	    $resource_tmp_count++;
	    // 檔案格式
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


	    // 編碼格式
	    if( isset( $distribution->characterSetCode) and !empty($distribution->characterSetCode) ){
	    	$resource->{NDCOP_API_RESOURCE_CHARACTER_SET_CODE}->set( $distribution->characterSetCode );
		}
	    // 資料項目最後更新時間
	    $resource_modified = isset( $dataset->field_version_update_time[LANGUAGE_NONE][0]['value'] ) ? $dataset->field_version_update_time[LANGUAGE_NONE][0]['value'] : $dataset->changed ;
	    $resource->{NDCOP_API_RESOURCE_MODIFIED}->set( $resource_modified );
	    
	    if( isset($distribution->downloadURL) and !empty($distribution->downloadURL) ){
	      // 資料項目網址型態
	      $resource->{NDCOP_API_RESOURCE_URLTYPE}->set( 'download' );
	      $field_collection_item->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][0]['url'] =  $distribution->downloadURL;
	    }
	    elseif( isset($distribution->accessURL) and !empty($distribution->accessURL) ){
	      // 資料項目網址型態
	      $resource->{NDCOP_API_RESOURCE_URLTYPE}->set( 'access' );
	      $field_collection_item->{NDCOP_API_RESOURCE_URL}[LANGUAGE_NONE][0]['url'] =  $distribution->accessURL;
	    }

	    // 資料項目對應政府詮釋資料集NID
	    if( isset( $dataset->nid ) and !empty( $dataset->nid ) ){
	    	$resource->{NDCOP_API_RESOURCE_NID_REF}->set( $dataset->nid );
		}
	    // APIIN的資料項目編號(從Batch進來)
	    if( isset( $resource_id ) and !empty( $resource_id ) ){
	    	$resource->{NDCOP_API_RESOURCE_APIIN_ID}->set( $resource_id );
		}
	    // Finally save our item
	    $field_collection_item->save();

  	}
  $dataset->{NDCOP_API_RESOURCE_COUNT}[LANGUAGE_NONE][0]['value'] = dechex($new_resource_count + 1);
  node_save($dataset);
}

function ndcop_api_resources_transfer_old_data_format(){
  $type_normal = array_flip( ndcop_api_get_taxonomy_term_names('data_normal') );
  $type_program = array_flip( ndcop_api_get_taxonomy_term_names('data_program') );
  $type_apps = array_flip( ndcop_api_get_taxonomy_term_names('data_apps') );
  $type_others = array_flip( ndcop_api_get_taxonomy_term_names('data_other') );
  $data_format = $type_normal+$type_program+$type_apps+$type_others;

  $data_format_machine_name = array();
  
  
  // 產生和dataset欄位有關的machine name array, ex: field_csv, field_xml...
  // key: tid; value: format machine name 
  foreach ($data_format as $tid => $format_name) {
          $data_format_machine_name[$tid] = 'field_'.strtolower($format_name);
  }
  return $data_format_machine_name;
}