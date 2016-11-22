<?php
/**
 * @file
 * 負責處理關鍵字欄位移轉.
 * TODO: 
 */


function ndcop_api_keywords_page(){
	$build = '';
	$unique_keywords = ndcop_api_keywords_list();
	drupal_set_message(print_r($unique_keywords, true)  );
	return $build;
}

function ndcop_api_keywords_get_old_keywords_field(){

	$query = db_select('node', 'node');
	$query->innerJoin(
	    'field_data_field_keyword',
	    'field_keyword',
	    'node.nid = field_keyword.entity_id AND field_keyword.entity_type = \'node\' AND field_keyword.deleted = \'0\' '
	);
	$query->condition('node.type', array('metadataset'), 'IN' );
	$query->addField('field_keyword','field_keyword_value','keyword');
	$query->addField('node','nid','nid');
	$results = $query->execute();
	return $results;
}

function ndcop_api_keywords_list(){
	
	$build = '';

	$keywords = array();

	$results = ndcop_api_keywords_get_old_keywords_field();

	foreach ($results as $key => $result) {
		$keyword_set = array();

		if( strpos($result->keyword, ',') ){
			$keyword_set = explode(',', $result->keyword);
		}
		elseif ( strpos($result->keyword, '，') ){
			$keyword_set = explode('，', $result->keyword);
		}
		elseif ( strpos($result->keyword, '、') ){
			$keyword_set = explode('、', $result->keyword);
		}
		elseif ( strpos($result->keyword, '；') ){
			$keyword_set = explode('；', $result->keyword);
		}
		elseif ( strpos($result->keyword, ';') ){
			$keyword_set = explode(';', $result->keyword);
		}
		else{
			if (!empty($keyword) and ($keyword !== ' ') ){
				$keywords[] =  trim($result->keyword) ;
			}
		}

		if (!empty($keyword_set)) {
			foreach ($keyword_set as $key => $keyword) {
				if (!empty($keyword) and ($keyword !== ' ') ){
					$keywords[] =  trim($keyword) ;
				}
			}

		}
	}
	$unique_keywords = array_unique($keywords);
	return $unique_keywords;
}

function ndcop_api_keywords_batch_form(){
	$form = array();

	$form['keywords']['import'] = array (
	    '#type' => 'submit',
	    '#value' => t('匯入關鍵字到tags分類'),
	    '#submit' => array('ndcop_api_keywords_batch_form_import_submit'),
	);

	$form['keywords']['mapping'] = array (
	    '#type' => 'submit',
	    '#value' => t('對應原關鍵字欄位到新的Taxonomy Reference欄位'),
	    '#submit' => array('ndcop_api_keywords_batch_form_mapping_submit'),
	);
	$form['keywords']['build_index'] = array (
	    '#type' => 'submit',
	    '#value' => t('對應完重建Taxonomy Index'),
	    '#submit' => array('ndcop_api_keywords_build_term_index_submit'),
	);

	$form['keywords']['delete'] = array (
	    '#type' => 'submit',
	    '#value' => t('移除tags分類的關鍵字'),
	    '#submit' => array('ndcop_api_keywords_batch_form_delete_submit'),
	);

	return $form;
}

function ndcop_api_keywords_batch_form_import_submit($form, $form_state){

	// 取得不重複關鍵字
	$unique_keywords = ndcop_api_keywords_list();

	// 批次處理新增Taxonomy term 
        $operations[] = array(
          'ndcop_api_create_keyword_taxonomy_term_batch_processing',
          array($unique_keywords),
        );

      // Once everything is gathered and ready to be processed... well... process it!
      $batch = array(
        'title' => t('匯入關鍵字中...'),
        'operations' => $operations,
        'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.keywords.process.inc',
        'finished' => 'ndcop_api_create_keyword_taxonomy_term_finished', 
        'error_message' => t('The installation has encountered an error.'),
        'progress_message' => t('處理 @current / @total 關鍵字'),
      );
      watchdog('ndcop api','%data', 
         array('%data' => '開始匯入關鍵字!', WATCHDOG_DEBUG));
      batch_set($batch);
}

function ndcop_api_keywords_batch_form_mapping_submit($form, $form_state){

	$num = 0;
	$keywords = array();
	$process_nids = array();
	
	$results = ndcop_api_keywords_get_old_keywords_field();
	// 取得Array key:term name, value:tid
	$vocabulary_name = 'tags';
	$taxonomy_keyword_terms = ndcop_api_get_taxonomy_term_names($vocabulary_name);

	foreach ($results as $key => $result) {
		echo $result->nid.'<br>';

		$keyword_set = array();
		if( strpos($result->keyword, ',') ){
			$keyword_set = explode(',', $result->keyword);
		}
		elseif ( strpos($result->keyword, '，') ){
			$keyword_set = explode('，', $result->keyword);
		}
		elseif ( strpos($result->keyword, '、') ){
			$keyword_set = explode('、', $result->keyword);
		}
		elseif ( strpos($result->keyword, '；') ){
			$keyword_set = explode('；', $result->keyword);
		}
		elseif ( strpos($result->keyword, ';') ){
			$keyword_set = explode(';', $result->keyword);
		}
		// 只有一個關鍵字
		else{
			// keyeord非空
			if (!empty($keyword) and ($keyword !== ' ') ){
				$keywords[] = $taxonomy_keyword_terms[trim($result->keyword)];
				echo trim($result->keyword).' '.$taxonomy_keyword_terms[trim($result->keyword)].'<br>';
				echo '----------------------------<br>';
			}
			else{
				echo 'Empty<br>';
				continue;
			}
		}

		// 可切割關鍵字
		if (!empty($keyword_set)) {
			// print_r($keyword_set);
			foreach ($keyword_set as $key => $keyword) {
				
				// keyeord非空
				if (!empty($keyword) and ($keyword !== ' ') ){
					$keywords[] = $taxonomy_keyword_terms[trim($keyword)];
					echo trim($keyword).' '.$taxonomy_keyword_terms[trim($keyword)].'<br>';
				}
			}

			echo '----------------------------<br>';
		}

		// 批次處理Node
		$process_nids[$result->nid] = $keywords;
		$num++;
		if( ($num % 200) == 0 ){
	        $operations[] = array(
	          'ndcop_api_create_dataset_keyword_term_reference_batch_processing',
	          array($process_nids),
	        );
	        $process_nids = array();
    	}

		// Reset
        $keywords = array();
	}
	// 剩下未滿200筆的nids
    $operations[] = array(
	          'ndcop_api_create_dataset_keyword_term_reference_batch_processing',
	          array($process_nids),
	        );

	// Once everything is gathered and ready to be processed... well... process it!
      $batch = array(
        'title' => t('更新資料集關鍵字欄位中...'),
        'operations' => $operations,
        'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.keywords.process.inc',
        'finished' => 'ndcop_api_create_dataset_keyword_term_reference_finished',
        'error_message' => t('The installation has encountered an error.'),
        'progress_message' => t('處理 @current / @total 資料集'),
      );
      watchdog('ndcop api','%data', 
         array('%data' => '開始更新資料集關鍵字欄位!', WATCHDOG_DEBUG));
      batch_set($batch);
}

function ndcop_api_keywords_batch_form_delete_submit($form, $form_state){
	$count = 0;
	$vocabulary_obj =  taxonomy_vocabulary_machine_name_load('tags');
	foreach(taxonomy_get_tree($vocabulary_obj->vid) as $term) { 
		taxonomy_term_delete($term->tid);
		$count++; 
	}
	drupal_set_message('移除關鍵字個數: %count', array('%count'=>$count));
}

function ndcop_api_create_keyword_taxonomy_term_batch_processing($unique_keywords, &$context){

	if (!isset($context['results']['created']) && !isset($context['results']['nocreate']) ) {
    	$context['results']['created'] = 0;
    	$context['results']['nocreate'] = array();
  	}
  	foreach ($unique_keywords as $key => $keyword) {
	  	watchdog('ndcop api','%id: %data, %item', 
	         array('%id'=>$key,'%data' => '匯入的關鍵字','%item'=>$keyword, WATCHDOG_DEBUG));

		$vocabulary_obj =  taxonomy_vocabulary_machine_name_load('tags');
		$term_obj = taxonomy_get_term_by_name($keyword, 'tags');

		// 檢查重複;若未重複則新增
		if( empty($term_obj) ){
			$term = new stdClass();
			$term->name = $keyword;
			$term->vid = $vocabulary_obj->vid;
			taxonomy_term_save($term);

			$context['results']['created']++;
		}
		else{
			$context['results']['nocreate'][] = $keyword;
		}
	}
}

function ndcop_api_create_keyword_taxonomy_term_finished($success, $results, $operations){

	if($success){
		$message = t('新增資料集關鍵字 @count 筆', array('@count' => $results['created'])).'<br />';
		$message .= t('未新增的關鍵字 @k', array('@k' => print_r($results['nocreate'],true))).'<br />';
		drupal_set_message($message);
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

function ndcop_api_create_dataset_keyword_term_reference_batch_processing($nids, &$context){

	if ( !isset($context['results']['created']) ) {
    	$context['results']['created'] = 0;
  	}
  	
  	foreach ($nids as $nid => $keywords) {
		watchdog('ndcop api','%data, %nid,%item', 
         array('%data' => '匯入的關鍵字','%nid'=>$nid,'%item'=>print_r($keywords, true), WATCHDOG_DEBUG));

		foreach ($keywords as $delta => $keyword) {
			$keyword_tid = $keyword;
			db_insert('field_data_'.FIELD_KEYWORD)
	    		->fields(
	    			array(
	    				'entity_type'=> 'node',
	    				'bundle'=> 'metadataset',
	    				'deleted'=> 0,
	    				'entity_id'=> $nid,
	    				'revision_id' => $nid,
	    				'language' => LANGUAGE_NONE,
	    				'delta' => $delta,
	    				FIELD_KEYWORD.'_tid' => $keyword_tid 
	    			)
	    		)
	    		->execute();
	    	db_insert('field_revision_'.FIELD_KEYWORD)
	    		->fields(
	    			array(
	    				'entity_type'=> 'node',
	    				'bundle'=> 'metadataset',
	    				'deleted'=> 0,
	    				'entity_id'=> $nid,
	    				'revision_id' => $nid,
	    				'language' => LANGUAGE_NONE,
	    				'delta' => $delta,
	    				FIELD_KEYWORD.'_tid' => $keyword_tid 
	    			)
	    		)
	    		->execute();
		}
		if($nid == 6818 or $nid == 9536 or $nid == 9725){
			continue;
		}
		$dataset = node_load($nid,null,true);
		watchdog('ndcop api','%data', 
         		array('%data' => '重建Node的nid '.print_r($dataset,true), WATCHDOG_DEBUG));
		taxonomy_delete_node_index($dataset);
  		taxonomy_build_node_index($dataset);
		$context['results']['created']++;
	}
}

function ndcop_api_create_dataset_keyword_term_reference_finished($success, $results, $operations){

	if($success){
		$message = t('新增資料集 @count 筆', array('@count' => $results['created'])).'<br />';
		drupal_set_message($message);
		watchdog('ndcop api','%data', 
         array('%data' => '更新資料集關鍵字欄位完成!', WATCHDOG_DEBUG));
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

function ndcop_api_keywords_build_term_index_submit(){

	watchdog('ndcop api','%data', 
         array('%data' => '開始重建Taxonomy Index!', WATCHDOG_DEBUG));
		$node_query = new EntityFieldQuery();

		$node_query->entityCondition('entity_type', 'node')
		  ->entityCondition('bundle', 'metadataset');

		$result = $node_query->execute();

		// Generate dataset identifiers
		if (isset($result['node'])) {

			  $identifiers = array();
			  $dataset_items_nids = array_keys($result['node']);

			  // 批次處理重建Taxonomy term index
	        $operations[] = array(
	          'ndcop_api_build_term_index_batch_processing',  // The function to run on each row
	          array($dataset_items_nids),  // The row in the excel
	        );
		      // Once everything is gathered and ready to be processed... well... process it!
		      $batch = array(
		        'title' => t('匯入關鍵字中...'),
		        'operations' => $operations,
		        'file' => drupal_get_path('module', 'ndcop_api') . '/ndcop_api.keywords.process.inc',
		        'finished' => 'ndcop_api_build_term_index_finished',
		        'error_message' => t('The installation has encountered an error.'),
		        'progress_message' => t('處理 @current / @total 資料集'),
		      );
		      watchdog('ndcop api','%data', 
		         array('%data' => '開始重建Taxonomy Term Index!', WATCHDOG_DEBUG));
		      batch_set($batch);
			}
}

function ndcop_api_build_term_index_batch_processing($nids, &$context){

	if ( !isset($context['results']['created']) ) {
    	$context['results']['created'] = 0;
  	}
  	
  	foreach ($nids as $nid) {
  		
		if($nid == 6818){
			continue;
		}
		// 建立taxonomy index,瀏覽分類頁面才會看到
		$dataset = node_load($nid,null,true);
		watchdog('ndcop api','%data', 
         		array('%data' => '重建Node的nid '.print_r($dataset,true), WATCHDOG_DEBUG));
		taxonomy_delete_node_index($dataset);
  		taxonomy_build_node_index($dataset);
		$context['results']['created']++;
	}
	
	
}

function ndcop_api_build_term_index_finished($success, $results, $operations){

	if($success){
		$message = t('重建資料集 @count 筆', array('@count' => $results['created'])).'<br />';
		drupal_set_message($message);
		watchdog('ndcop api','%data', 
         array('%data' => '重建Taxonomy Term Index完成!', WATCHDOG_DEBUG));
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

function ndcop_api_create_keyword_taxonomy_term($keyword){
	$vocabulary_obj =  taxonomy_vocabulary_machine_name_load('tags');
	$term_obj = taxonomy_get_term_by_name($keyword);

	// 檢查重複;若未重複則新增
	if( empty($term_obj) ){
		$term = new stdClass();
		$term->name = $keyword;
		$term->vid = $vocabulary_obj->vid;
		taxonomy_term_save($term);
		return TRUE;
	}
	else{
		return FALSE;
	}
}