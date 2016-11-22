<?php

/**
 * Builds a form that will allow users to upload csv files
 * 
 * @see
 *   hook_menu()
 */
function ndcop_api_id_mapping_process_export_form($form, &$form_state) {
	
	$form = array();

	// 產生機關選單
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'taxonomy_term')
	  	->entityCondition('bundle', array('org'));
	$result = $query->execute();
	$org_tids = array_keys($result['taxonomy_term']);

	module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.pages');
	$tid_name_mapping = ndcop_dataset_api_org_tid_name_mapping( $org_tids );
	
	$form['filter']= array(
		'#type' => 'fieldset',
		'#attributes' => array('class' => array('areaBlue')),
	);
 	$form['filter']['filter_org'] = array(
    	'#type' => 'select',
    	'#title' => t('選擇匯出機關'),
    	'#options' => $tid_name_mapping,
    	'#required' => true,
    	'#label_attributes' => array('class' => array('title') ),
  	);

  	$form['filter']['ndcop_api_mapping_export_fieldset'] =array( 
    	'#type' => 'submit', 
    	'#value' => t('匯出機關暨其所屬資料集'),
    	'#submit' => array('ndcop_api_id_mapping_export_batch_submit'),
  	);

  	// 匯出Batch完成，下載匯出檔案
  	if( isset( $_SESSION['ndcop_api']['export']['filename'] ) ){
    	$filename = $_SESSION['ndcop_api']['export']['filename'];
    	// 產生下載連結路徑
    	$path = file_create_url( file_build_uri($filename) );
    	// 讓下載連結路徑可以讓js使用
    	drupal_add_js(array('export_file_info' => array('downloadpath' => $path)), 'setting');
    	// 呼叫下載js
    	drupal_add_js(drupal_get_path('module', 'batch_import') . '/batch_plugin.js');
    	// 移除下載路徑$_SESSION
    	unset($_SESSION['ndcop_api']['export']['filename']);
  	}

	return $form;
}

/*
 * @匯出
 * 利用Batch API匯出的submit callback function
 */
function ndcop_api_id_mapping_export_batch_submit($form, &$form_state){

	$filter_org = taxonomy_term_load( $form_state['values']['filter_org'] );
	$filter_oid = $filter_org->field_orgoid[LANGUAGE_NONE][0]['value'];

	$rows = array();
	// 產生詮釋資料匯出標頭
  	$dataset_header = ndcop_api_id_mapping_process_export_header();
  	// 取得匯出資料集NID
  	$results = ndcop_api_id_mapping_process_get_dataset_nids( $filter_oid );

  	foreach ($results as $result){
  		$rows[] = array('', $result->identifier, $result->title, $result->nid);
  	}

  	$total_row_count = count($rows);
	$split_rows = array_chunk($rows, 5);
	foreach ($split_rows as $key => $rows) {
	        
	    $operations[] = array(
	      'ndcop_api_id_mapping_export_batch_processing',
	      array($rows, $dataset_header, $total_row_count),
	    );
	}
	$batch = array(
	    'title' => t('匯出資料集中...'),
	    'operations' => $operations,
	    'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.identifier.mapping.inc',
	    'finished' => 'ndcop_api_id_mapping_export_batch_processing_finished',
	    'error_message' => t('The installation has encountered an error.'),
	    'progress_message' => t('資料集匯出中...'),
	  );
	watchdog('ndcop_api','%data', 
	         array('%data' => '開始匯出資料集!', WATCHDOG_DEBUG));
	batch_set($batch);
}

/**
 * Builds a form that will allow users to upload csv files
 * 
 * @see
 *   hook_menu()
 */
function ndcop_api_id_mapping_process_import_form($form, &$form_state) {
	
	$form = array();

	$form['dataset_batch_import_fieldset']['import_container']['import'] = array(
	    '#title' => t('檔案上傳'),
	    '#type' => 'managed_file',
	    '#upload_location' => 'public://tmp/',
	    '#upload_validators' => array(
	      'file_validate_extensions' => array('xls xlsx'),
	    ),
	);

  	$form['dataset_batch_import_fieldset']['import_container']['submit'] = array (
    	'#type' => 'submit',
    	'#value' => t('匯入資料集序號對照表'),
    	'#states' => array(
      	'invisible' => array(':input[name="import[fid]"]' => array('value' =>0),),
    	)
  	);

	return $form;
}

/**
 * @匯入
 * Submit handler for batch_import_import_form()
 */
function ndcop_api_id_mapping_process_import_form_submit($form, &$form_state) {

	// Check to make sure that the file was uploaded to the server properly
  	$uri = db_query("SELECT uri FROM {file_managed} WHERE fid = :fid", array(':fid' => $form_state['input']['import']['fid'],
  	))->fetchField();

  	if(!empty($uri)) {
  		if(file_exists(drupal_realpath($uri))) {
  			// 開始從EXCEL讀取檔案
      		$handle = fopen(drupal_realpath($uri), "r");
      
      		$PHPExcel = new PHPExcel (); 
      
      		$PHPReader = new PHPExcel_Reader_Excel2007 (); 
      		if (! $PHPReader->canRead ( drupal_realpath($uri) )) {
        		$PHPReader = new PHPExcel_Reader_Excel5 (); 
        		if (! $PHPReader->canRead ( drupal_realpath($uri) )) { 
          			return; 
        		} 
      		}
      		$PHPExcel = $PHPReader->load ( drupal_realpath($uri) );

      		$currentSheet = $PHPExcel->getSheet ( 0 ); 
      		//取得有多少column(英文)
      		$allColumn = $currentSheet->getHighestColumn();
      
      		//取得有多少column(數字) 
      		$col = PHPExcel_Cell::columnIndexFromString($currentSheet->getHighestColumn()); 
      		//取得有多少row 
      		$allRow = $currentSheet->getHighestRow();

      		$data = array();
      		$empty_col =0;
      		$i = 0;
      		for($rowIndex = 2; $rowIndex <= $allRow; $rowIndex++) { 
        		for($colIndex = 0; $colIndex < $col; $colIndex++) {
          			$cell = $currentSheet->getCellByColumnAndRow($colIndex, $rowIndex)->getValue(); 
          			$cell = trim($cell);
          			if(empty($cell))
            			$empty_col++;
          			$isDate = PHPExcel_Shared_Date::isDateTime($currentSheet->getCellByColumnAndRow($colIndex, $rowIndex));
          			if($isDate){
            			$data[$i][] = (!empty($cell)) ? PHPExcel_Shared_Date::ExcelToPHP( $cell ) : '' ;
          			}
          			else{
            			$data[$i][] = $cell;
          			}
        		}
        		if($col == $empty_col){
          			unset($data[$i]);
          			break;
        		}
        		$empty_col =0;
        		$i++;
      		}

      		//從EXCEL讀取檔案結束

      		watchdog('ndcop_api','%data, %item', array('%data' => '匯入EXCEL所有的Records','%item'=>print_r($data,true), WATCHDOG_DEBUG));
      		fclose($handle);

      		// 批次處理
      		$split_rows = array_chunk($data, 50);

      		foreach ($split_rows as $key => $rows) {
        		$operations[] = array(
      				'ndcop_api_id_mapping_import_batch_processing',
      				array($rows),
	    		);
			}
	      	// Once everything is gathered and ready to be processed... well... process it!
			$batch = array(
	    		'title' => t('資料集序號對應中...'),
	    		'operations' => $operations, 
	    		'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.identifier.mapping.inc',
	    		'finished' => 'ndcop_api_id_mapping_import_batch_processing_finished',
	    		'error_message' => t('The installation has encountered an error.'),
	    		'progress_message' => t('資料集序號對應中...'),
	  		);
			batch_set($batch);

  		}
  	}	else {
    	drupal_set_message(t('您尚未上傳檔案或是檔案上傳發生錯誤。'), 'error');
  	}
}

/**
 *  跑Drupal batch對應資料集序號
 *  @see ndcop_api_id_mapping_import_batch_submit
 *  @param $rows : 匯入資料列
 */
function ndcop_api_id_mapping_import_batch_processing( $rows, &$context ){
	
	watchdog('ndcop_api','%data, %item', array('%data' => '欲對應的Records','%item'=>print_r($rows,true), WATCHDOG_DEBUG));

	//設置使用者訊息(更新幾筆,失敗幾筆,現在第幾筆)
   	if (!isset($context['results']['updated'], $context['results']['error'],$context['results']['now'])) {
    	$context['results']['updated'] = 0;
    	$context['results']['error'] = 0;
    	$context['results']['now'] = 2;
  	}

  	foreach ($rows as $idx => $row) {
  		// 介接平臺資料集序號不為空
  		if ( !empty($row[0]) and !empty($row[3]) ) {
  			// 更新政府資料開放平臺中介接平臺相關資料集序號
  			$exchange_system_id = $row[0];
  			$ndc_nid = $row[3];
  			$result = ndcop_api_id_mapping_action_apiin( $exchange_system_id, $ndc_nid );
        $result = ndcop_api_id_mapping_action_ndc( $exchange_system_id, $ndc_nid );

  			// 處理ndc_nid相對應資料資源中的APIIN的資料資源編號(field_resource_in_id_g)
  			$result = ndcop_api_id_mapping_action_resource( $ndc_nid, $exchange_system_id );

        $context['results']['updated']++;
  		}
  		else{
  			$error_index = $context['results']['now'];
      		$context['results']['error_msg'][$error_index] = '介接平臺資料集序號為空或政府資料開放平臺系統代號(nid)為空
';
  			$context['results']['error']++;
  		}

  		$context['results']['now']++;
  	}
}

/**
 *  跑Drupal batch對應資料集序號finish function
 */
function ndcop_api_id_mapping_import_batch_processing_finished( $success, $results, $operations ){

  if($success){

    // 使用到db_update和db_insert直接修改node在資料庫的資料，所以需要清cache，保持node_load的正確性
    drupal_flush_all_caches();

      // 顯示匯出訊息
    $message = t('匯入對應資料集序號 @count 筆', array('@count' => $results['updated'] )).'<br />';
    drupal_set_message($message);
     //處理錯誤訊息
    if($results['error'] > 0){
      drupal_set_message(t('以下 @count 筆資料有誤，請更正後再匯入', array('@count' => $results['error']) ),'warning');

      foreach ($results['error_msg'] as $row_num=>$msg) {

        $msg_num .= '第'.$row_num.'筆=><br>';
        if(is_array($msg)){
              $v_msg = $msg_num;
              foreach ($msg as $validate_msg) {
                $v_msg .= t('@validate_msg', array('@validate_msg' => $validate_msg)).'<br>';
                
              }
              drupal_set_message($v_msg,'warning');
        }
        else{
              
              $msg = $msg_num.$msg;
              drupal_set_message($msg,'warning');
        }
        $msg_num = '';
      }
    }
    
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

/**
 *  跑Drupal batch匯出產生檔案
 *  @see ndcop_api_id_mapping_export_batch_submit
 *  @param $rows : 匯出資料列
 *  @param $headers : 匯出資料標頭
 *  @param $total_row_count : 匯出資料列全部列數
 */
function ndcop_api_id_mapping_export_batch_processing( $rows, $headers, $total_row_count, &$context ){

	if ( !isset($context['results']['objPHPExcel']) and !isset($context['results']['rows_count']) ) {

		$letters = array('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z');

		$objPHPExcel = new PHPExcel();
	  	$objPHPExcel->setActiveSheetIndex(0);
	  	$objPHPExcel->getActiveSheet()->setTitle('匯出資料集');

	  	// 依照$headers動態產生EXCEL欄位標題
	  	$header_count = count($headers);
	  	$letters_count = count($letters);
	  	foreach ( $headers as $key => $header) {
	  		$first_index = intval($key / $letters_count) - 1;
	  		$second_index = ($key % $letters_count);
	  		if( $first_index == (-1) ){
	  			$cell = $letters[$second_index] . '1';
	  		}
	  		else{
	  			$cell = $letters[$first_index] . $letters[$second_index] . '1';
	  		}

	  		watchdog('ndcop_api','%data, %cell, %header, %first_index, %$second_index', array('%data' => '產生EXCEL欄位標題!','%cell'=>$cell, '%header'=>$header['data'], '%first_index' => $first_index, '%$second_index' => $second_index ), WATCHDOG_DEBUG);
	  		$objPHPExcel->setActiveSheetIndex(0)->setCellValue($cell , $header['data'] );
	  	}
    	$context['results']['objPHPExcel'] = $objPHPExcel;
    	$context['results']['rows_count'] = 2;

    	// 第一次Batch
    	foreach( $rows as $row ){

  			// 欄位數
  			for ($i = 0; $i < count($headers); $i++) {
  				if( isset($row[$i]) and !empty($row[$i]) ){
  					$objPHPExcel->getActiveSheet()->setCellValueByColumnAndRow($i, $context['results']['rows_count'], $row[$i]  );
  				}
  			}

  			$context['results']['rows_count']++;
  		}

  	}
  	else{
		// 第一次Batch之外，其他Batch
		$objPHPExcel = $context['results']['objPHPExcel'];	
		foreach( $rows as $row ){

			// 欄位數
			for ($i = 0; $i < count($headers); $i++) {
				if( isset($row[$i]) and !empty($row[$i]) ){ 
				$objPHPExcel->getActiveSheet()->setCellValueByColumnAndRow($i, $context['results']['rows_count'], $row[$i]  );
				}
			}

			$context['results']['rows_count']++;
		}
  	}
  	
  	// Show message updating user on how many subscribers have been exported.
  $context['message'] = t('處理 @count / @total 資料集', array(
    '@count' => $context['results']['rows_count']-2,
    '@total' => $total_row_count,
  ));
}

/**
 *  跑Drupal batch匯出產生檔案finish function
 */
function ndcop_api_id_mapping_export_batch_processing_finished( $success, $results, $operations ){

	if($success){

		// 產生匯出資料
		$objPHPExcel = $results['objPHPExcel'];
	    $objWriter = new PHPExcel_Writer_Excel2007($objPHPExcel);

	    $dir = file_stream_wrapper_get_instance_by_uri('public://')->realpath();

	    $filename = '匯出機關暨其所屬資料集序號對照_'. time() .'.xlsx';
	    $path = $dir.'/'.$filename;
	  
	    $objWriter->save($path);

	    // 顯示匯出訊息
		$message = t('匯出資料集 @count 筆', array('@count' => $results['rows_count']-2 )).'<br />';
		drupal_set_message($message);
		watchdog('batch_import','%data, 路徑 %path', array('%data' => '匯出資料集完成!', '%path'=>$path, WATCHDOG_DEBUG));

		// 設定SESSION
		$_SESSION['ndcop_api']['export']['filename'] = $filename;
		
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

/**
 *  產生詮釋資料匯出標頭
 *  @see
 */
function ndcop_api_id_mapping_process_export_header(){

	$header = array(
		array( 'data'=>'介接平臺資料集序號'),
		array( 'data'=>'政府資料開放平臺資料集序號'),
		array( 'data'=>'資料集名稱'),
    array( 'data'=>'政府資料開放平臺系統代號(nid)'),
	);

	return $header;
}

/**
 *  取得匯出資料集NID
 *  @see ndcop_api_id_mapping_export_batch_submit
 *  @param $filter_oid
 */
function ndcop_api_id_mapping_process_get_dataset_nids( $filter_oid ){

	// 機關暨其所屬資料集
  	$query = db_select('node','node');

  	$query->leftJoin('field_data_field_oid', 'field_data_field_oid', 
          'node.nid = field_data_field_oid.entity_id 
          AND (field_data_field_oid.entity_type = \'node\' AND field_data_field_oid.deleted = \'0\') ');

  	$query->leftJoin('field_data_field_serialno', 'field_data_field_serialno', 
          'node.nid = field_data_field_serialno.entity_id 
          AND (field_data_field_serialno.entity_type = \'node\' AND field_data_field_serialno.deleted = \'0\') ');

  	$query->join('field_data_field_dataset_status', 'field_data_field_dataset_status', 
          'node.nid = field_data_field_dataset_status.entity_id 
          AND (field_data_field_dataset_status.entity_type = \'node\' AND field_data_field_dataset_status.deleted = \'0\')');

  	$query->condition('node.type', array('metadataset'),'IN')
        ->condition('field_data_field_oid.field_oid_value', db_like($filter_oid) . '%', 'LIKE');

  	$query->fields('node', array('nid', 'title'));
  	$query->addField('field_data_field_serialno','field_serialno_value','identifier');
  	$results = $query->execute();

  	return $results;
}

/** 
 * 更新政府資料開放平臺中介接平臺相關資料集序號(field_dataset_in_id_g)
 * @param $exchange_system_id: 介接平臺資料集序號
 * @param $ndc_nid: 政府資料開放平臺系統代號(nid)
 */
function ndcop_api_id_mapping_action_apiin( $exchange_system_id, $ndc_nid ){

	$query = db_select('field_data_field_dataset_in_id_g','apiin_id');
	$query->condition('apiin_id.entity_id', $ndc_nid);
	$num_rows = $query->countQuery()->execute()->fetchField();
	$num_rows = intval($num_rows);
	try{
		if( $num_rows == 1 ){
			// 找到一筆，更新
			db_update('field_data_field_dataset_in_id_g') 
				->fields(array(
					'field_dataset_in_id_g_value' => $exchange_system_id,
				))
				->condition('entity_id', $ndc_nid)
				->execute();
			db_update('field_revision_field_dataset_in_id_g') 
				->fields(array(
					'field_dataset_in_id_g_value' => $exchange_system_id,
				))
				->condition('entity_id', $ndc_nid)
				->execute();
			return true;
		}
		elseif( $num_rows == 0 ){
			// 找不到，新增
			db_insert('field_data_field_dataset_in_id_g')->fields(
					 	array(
					 	'entity_type'=> 'node',
					 	'bundle'=> 'metadataset',
					 	'deleted'=> 0,
					 	'entity_id' => $ndc_nid,
					 	'revision_id' => $ndc_nid,
					 	'language'=> LANGUAGE_NONE,
					 	'delta'=> 0,
					 	'field_dataset_in_id_g_value' => $exchange_system_id
					 	)
			)->execute();

			db_insert('field_revision_field_dataset_in_id_g')->fields(
					 	array(
					 	'entity_type'=> 'node',
					 	'bundle'=> 'metadataset',
					 	'deleted'=> 0,
					 	'entity_id' => $ndc_nid,
					 	'revision_id' => $ndc_nid,
					 	'language'=> LANGUAGE_NONE,
					 	'delta'=> 0,
					 	'field_dataset_in_id_g_value' => $exchange_system_id
					 	)
			)->execute();
			return true;
		}
		else{
			// 有重複
			throw new Exception("更新政府資料開放平臺中介接平臺相關資料集序號(field_dataset_in_id_g)發現重複");
			return false;
			
		}
	}catch(Exception $e){
				echo ($e->getMessage());
				drupal_set_message($e->getMessage(), 'error');
        		return null;
	}
}

/** 
 * 更新政府資料開放平臺中介接平臺相關資料集序號(field_serialno)
 * @param $exchange_system_id: 介接平臺資料集序號
 * @param $ndc_nid: 政府資料開放平臺系統代號(nid)
 */
function ndcop_api_id_mapping_action_ndc( $exchange_system_id, $ndc_nid ){

  $query = db_select('field_data_field_serialno','serialno');
  $query->condition('serialno.entity_id', $ndc_nid);
  $num_rows = $query->countQuery()->execute()->fetchField();
  $num_rows = intval($num_rows);

  try{

    if( $num_rows == 1 ){
      // 找到一筆，更新
      db_update('field_data_field_serialno') 
        ->fields(array(
          'field_serialno_value' => $exchange_system_id,
        ))
        ->condition('entity_id', $ndc_nid)
        ->execute();
      db_update('field_revision_field_serialno') 
        ->fields(array(
          'field_serialno_value' => $exchange_system_id,
        ))
        ->condition('entity_id', $ndc_nid)
        ->execute();
      return true;
    }

  }catch(Exception $e){
        echo ($e->getMessage());
        drupal_set_message($e->getMessage(), 'error');
            return null;
  }
}

/** 
 * 處理ndc_nid相對應資料資源中的APIIN的資料資源編號(field_resource_in_id_g)
 * @param $ndc_nid: 政府資料開放平臺系統代號(nid)
 * @param $exchange_system_id: 介接平臺資料集序號
 */
function ndcop_api_id_mapping_action_resource( $ndc_nid, $exchange_system_id ){

	$dataset = node_load( $ndc_nid, null, true );

	foreach( $dataset->{NDCOP_API_RESOURCE}[LANGUAGE_NONE] as $fc){

	  $field_collection_item_id = $fc['value'];
	  $field_collection_item = field_collection_item_load($field_collection_item_id);

    $orig_rsc_id = $field_collection_item->field_resource_id_g[LANGUAGE_NONE][0]['value'];

	  if( empty($field_collection_item->field_resource_in_id_g[LANGUAGE_NONE][0]['value']) ){

	  	// 若APIIN的資料資源編號為空則設定為資料資源編號的值
		  $field_collection_item->field_resource_in_id_g[LANGUAGE_NONE][0]['value'] = ndcop_api_id_mapping_new_resource_id( $orig_rsc_id, $exchange_system_id );

      // 更新原有資料資源編號的值
      $field_collection_item->field_resource_id_g[LANGUAGE_NONE][0]['value'] = ndcop_api_id_mapping_new_resource_id( $orig_rsc_id, $exchange_system_id );
	  }
    else{
      
      // 更新原有資料資源編號的值
      $field_collection_item->field_resource_id_g[LANGUAGE_NONE][0]['value'] = ndcop_api_id_mapping_new_resource_id( $orig_rsc_id, $exchange_system_id );
      
      $field_collection_item->field_resource_in_id_g[LANGUAGE_NONE][0]['value'] = ndcop_api_id_mapping_new_resource_id( $orig_rsc_id, $exchange_system_id );
    }

      // save要帶參數TRUE，才不會影響原本的host
      $field_collection_item->save(TRUE);   

	}
}

/** 
 * 利用匯入介接平臺資料集序號產生新的10+6的資料資源編號(field_resource_in_id_g)
 * @param $orig_rsc_id: 跨平臺前置作業匯入前原有資料資源編號
 * @param $exchange_system_id: 介接平臺資料集序號
 */
function ndcop_api_id_mapping_new_resource_id( $orig_rsc_id, $exchange_system_id ){

  $split_term = array();

  try{
    $split_term = explode("-", $orig_rsc_id);
    if( !empty($split_term[2]) ){
      return $exchange_system_id . '-' . $split_term[2];
    }
    else{
      throw new Exception("原有資料資源編號切割失敗");
    }

  }catch(Exception $e){
        watchdog('ndcop_api','跨平臺前置作業: %err_msg', 
           array('%err_msg' => $e->getMessage(), WATCHDOG_DEBUG));
        return null;
  }
}