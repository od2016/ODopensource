<?php
/**
 * @file
 * 負責處理資料集序號欄位轉換.
 * TODO: 
 */


/**
 * 平臺上目前資料集序號展示頁面.
 */
function ndcop_api_serialno_page(){
	$build = '';
	$num = 0;
	$results = ndcop_api_get_serialno_field();

	foreach ($results as $result) {
		echo $result->nid.' :'.$result->serialno.'<br>';
		$new_serialno = ndcop_api_serialno_transform($result->serialno);
		echo $new_serialno.'<br>';
		$num++;
	}
	echo 'Count: '.$num;
	return $build;
}

function ndcop_api_serialno_batch_form(){

	$form = array();

	$form['serialno']['import'] = array (
	    '#type' => 'submit',
	    '#value' => t('修改資料集序號五碼至六碼'),
	    '#submit' => array('ndcop_api_serialno_batch_form_submit'),
	);

	return $form;
}

function ndcop_api_serialno_batch_form_submit($form, &$form_state){
	// 取得平臺上資料集序號.
	$results = ndcop_api_get_serialno_field();

	$num = 0;
	$process_nids = array();
	// 批次處理資料集序號轉換
	foreach ($results as $result) {
        
        // 五碼轉六碼
		$new_serialno = ndcop_api_serialno_transform($result->serialno);

		$process_nids[$result->nid] = $new_serialno;
		$num++;
		if( ($num % 100) == 0 ){
	        $operations[] = array(
	          'ndcop_api_create_serialno_batch_processing',
	          array($process_nids),
	        );
	        $process_nids = array();
    	}
    }
    $operations[] = array(
	          'ndcop_api_create_serialno_batch_processing',
	          array($process_nids),
	        );
      // Once everything is gathered and ready to be processed... well... process it!
      $batch = array(
        'title' => t('轉換資料集序號中...'),
        'operations' => $operations,
        'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.serialno.process.inc',
        'finished' => 'ndcop_api_create_serialno_finished',
        'error_message' => t('The installation has encountered an error.'),
        'progress_message' => t('處理 @current / @total 資料集序號'),
      );
      watchdog('ndcop api','%data', 
         array('%data' => '開始轉換資料集序號!', WATCHDOG_DEBUG));
      batch_set($batch);
}

function ndcop_api_create_serialno_batch_processing($nids, &$context){

	if ( !isset($context['results']['created']) ) {
    	$context['results']['created'] = 0;
  	}

  	foreach ($nids as $nid => $serialno) {
  		watchdog('ndcop api','%data, %nid,%item', 
         array('%data' => '轉換的新資料集序號','%nid'=>$nid,'%item'=>$serialno, WATCHDOG_DEBUG));
		db_update('field_data_'.FIELD_SERIALNO)
    		->fields(
    			array(
    				FIELD_SERIALNO.'_value' => $serialno 
    			)
    		)
    		->condition('entity_id', $nid)
    		->execute();
    	db_update('field_revision_'.FIELD_SERIALNO)
    		->fields(
    			array(
    				FIELD_SERIALNO.'_value' => $serialno 
    			)
    		)
    		->condition('entity_id', $nid)
    		->execute();

    		$context['results']['created']++;
  	}
  	
	
	
	
}

function ndcop_api_create_serialno_finished($success, $results, $operations){
	if($success){
		$message = t('轉換資料集序號 @count 筆', array('@count' => $results['created'])).'<br />';
		drupal_set_message($message);
		watchdog('ndcop api','%data', 
         array('%data' => '轉換資料集序號欄位完成!', WATCHDOG_DEBUG));
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
 * 資料集序號五碼轉六碼.
 */
function ndcop_api_serialno_transform($serialno){

	$serialno_parts = explode('-',$serialno);
	if(strlen($serialno_parts[1]) == 5){
		$new_serialno = $serialno_parts[0].'-0'.$serialno_parts[1];
	}
	else{
		$new_serialno = $serialno;
	}
	return $new_serialno;
}

/**
 * 回傳平臺上資料集序號.
 */
function ndcop_api_get_serialno_field(){

	$query = db_select('node', 'node');
	$query->innerJoin(
	    'field_data_field_serialno',
	    'field_serialno',
	    'node.nid = field_serialno.entity_id AND field_serialno.entity_type = \'node\' AND field_serialno.deleted = \'0\' '
	);
	$query->condition('node.type', array('metadataset'), 'IN' );
	$query->addField('field_serialno','field_serialno_value','serialno');
	$query->addField('node','nid','nid');
	$results = $query->execute();
	return $results;
}

function ndcop_api_update_serialno_field($nid, $serialno){

	$dataset = node_load($nid);
	print_r($dataset->field_serialno[LANGUAGE_NONE][0]['value']);
}