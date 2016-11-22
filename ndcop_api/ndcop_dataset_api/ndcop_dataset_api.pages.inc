<?php

/**
 * @file
 * 資料集詮釋資料API相關頁面.
 * TODO:
 */

/**
 * 機關發布與定期擷取設定畫面
 * @param
 */
function ndcop_dataset_api_exchang_setting_page(){

	$build = '';
	$build['exchang_setting_form'] = drupal_get_form('ndcop_dataset_api_exchang_setting_form');

	return $build;
}

/**
 * 個別機關發布與定期擷取設定畫面
 * @param $org_term: 機關Taxonomy term
 * @param $action: publish OR harvest
 */
function ndcop_dataset_api_exchang_org_setting_page($org_term, $action){

	$build = '';
	if($action == 'harvest'){
		drupal_set_title( drupal_get_title().'-定期擷取排程-'.$org_term->name );
		$build['harvest_setting_form'] = drupal_get_form('ndcop_dataset_api_exchang_org_harvest_setting_form', $org_term);
	}
	elseif ($action == 'publish') {
		drupal_set_title( drupal_get_title().'-機關IP與API Key-'.$org_term->name );
		$build['harvest_setting_form'] = drupal_get_form('ndcop_dataset_api_exchang_org_publish_setting_form', $org_term);
	}
	return $build;
}

/**
 * 跨平臺介接紀錄查詢畫面(後台管理者)
 * @param
 */
function ndcop_dataset_api_exchang_log_page( $backend_role ){

	$build = '';

	//define table header
	$header = array(
		array( 'data'=>'介接'),
		array( 'data'=>'操作'),
		array( 'data'=>'發布機關IP'),
		array( 'data'=>'時間', 'field'=>'req_time'),
		array( 'data'=>'執行結果'),
		array( 'data'=>'錯誤代碼'),
	    array( 'data'=>'內容'),
	);

	//Get SESSION
	$log_api_type = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_type'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_type'] : 'All';
	$log_api_publish_op = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_publish_op'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_publish_op'] : '';
	$log_api_ip = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_ip'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_ip'] : '';
	$log_api_result = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'] : -1;

	$filter_start = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'] : date("Y-m-d", strtotime('-1 week'));
	$filter_end = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'] : date("Y-m-d");

	$filter_condition = array(
		'filter_start'=> $filter_start,
		'filter_end' => $filter_end,
		'log_api_type' => $log_api_type,
		'log_api_publish_op' => $log_api_publish_op,
		'log_api_ip' => $log_api_ip,
		'log_api_result' => $log_api_result,
        'tablesort'=>TRUE,
        'pager'=>TRUE,
	);

	$results = ndcop_dataset_api_get_exchang_log($header, $filter_condition);

	//Build filter form
	$build['filter_form'] = drupal_get_form('ndcop_dataset_api_exchang_log_filter_form');

	foreach ($results as $result) {
		$api_type = ( strpos($result->request_method, 'Harvest' ) === false) ? 'Publish' : 'Harvest' ;
		$api_result = ( $result->success == 1 )? '成功' : '失敗' ;
		$rows[] =array(
			'data' => array(
				$api_type,
				$result->request_method,
				$result->request_ip,
				$result->req_time,
				$api_result,
				$result->err_type,
				$result->result,
			)
		);
	}

	$build['content'] = array(
		'#theme' =>'table',
		'#header' => $header,
		'#rows' => $rows,
		'#sticky' => true,
		'#empty' => '查無資料',
	);

	$build['pager'] = array(
		'#theme' => 'pager',
		'#weight' => 5
	);

	return $build;
}

/**
 * 跨平臺介接紀錄查詢畫面(部會管理者)
 * @param
 */
function ndcop_dataset_api_exchang_log_admin_author_page( $backend_role ){

	$build = '';

	$publisher_ip_set = array();
	$publisher_ip_set = ndcop_api_get_taxonomy_term_ip_info('org');
	$header = array(
		array( 'data'=>'介接'),
		array( 'data'=>'操作'),
		array( 'data'=>'發布機關IP'),
		array( 'data'=>'時間', 'field'=>'req_time'),
		array( 'data'=>'執行結果'),
		array( 'data'=>'錯誤代碼'),
       array( 'data'=>'內容'),
       );

	//Get SESSION
	$log_api_publish_op = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_publish_op'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_publish_op'] : '';
	$log_api_ip = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_ip'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_ip'] : '';
	$log_api_result = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'] : -1;

	$filter_start = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'] : date("Y-m-d", strtotime('-1 week'));
	$filter_end = isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'] : date("Y-m-d");

	$filter_condition = array(
		'filter_start'=> $filter_start,
		'filter_end' => $filter_end,
		'log_api_type' => 'Publish',
		'log_api_publish_op' => $log_api_publish_op,
		'log_api_ip' => $log_api_ip,
		'log_api_result' => $log_api_result,
        'tablesort'=>TRUE,
        'pager'=>TRUE,
        );
    foreach ($publisher_ip_set as $info) {
        if($info['oid'] === $_SESSION['account']['oid']){
            $publisherIpRaw = $info['ip_set'];
        }
    }
    $publisherIpList = explode(',', $publisherIpRaw);
    $results = ndcop_dataset_api_get_exchang_log($header, $filter_condition, $publisherIpList);

	//Build filter form
    $build['filter_form'] = drupal_get_form('ndcop_dataset_api_exchang_log_filter_form', $backend_role);

    $rows = array();
	// 每筆介接紀錄
    foreach ($results as $result) {
       $api_type = ( strpos($result->request_method, 'Harvest' ) === false) ? 'Publish' : 'Harvest' ;
       $api_result = ( $result->success == 1 )? '成功' : '失敗' ;
       $rows[] = array(
          'data' => array(
             $api_type,
             $result->request_method,
             $result->request_ip,
             $result->req_time,
             $api_result,
             $result->err_type,
             $result->result,
             )
          );
   }
   $build['content'] = array(
      '#theme' =>'table',
      '#header' => $header,
      '#rows' => $rows,
      '#sticky' => true,
      '#empty' => '查無資料',
      );

   $build['pager'] = array(
      '#theme' => 'pager',
      '#weight' => 5
      );

   return $build;
}

/**
 * 機關發布與定期擷取設定選擇表單
 * @param
 */
function ndcop_dataset_api_exchang_setting_form(){
	$form = array();

	// 產生機關選單
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'taxonomy_term')
	  	->entityCondition('bundle', array('org'));
	$result = $query->execute();
	$org_tids = array_keys($result['taxonomy_term']);
	$tid_name_mapping = ndcop_dataset_api_org_tid_name_mapping( $org_tids );

	$form['filter']= array(
		'#type' => 'fieldset',
		'#attributes' => array('class' => array('areaBlue')),
	);
 	$form['filter']['filter_org'] = array(
    	'#type' => 'select',
    	'#title' => t('選擇設定機關'),
    	'#options' => $tid_name_mapping,
    	'#required' => true,
    	'#label_attributes' => array('class' => array('title') ),
  	);

 	$form['filter']['settings']['action'] = array(
	    '#type' => 'radios',
	    '#title' => t('設定類型'),
	    '#options' => array(
	    	'publish' => '發布功能設定',
	    	'harvest' => '定期擷取設定'
	    ),
	    '#required' => true,
	    '#label_attributes' => array('class' => array('title') ),
	  );

  	$form['filter']['submit'] = array(
	    '#type'=>'submit',
    	'#value' => t('查詢'),
    	'#attributes' => array('style' => 'margin-top:25px;'),
  	);

	return $form;
}

/**
 * 機關發布與定期擷取設定選擇表單 Submit Callback
 * @param
 */
function ndcop_dataset_api_exchang_setting_form_submit( $form, &$form_state ){

	global $user;
	$config_org_tid = $form_state['values']['filter_org'];
	$action = $form_state['values']['action'];
	$form_state['redirect'] = 'apiexchange/config/'. $config_org_tid .'/' . $action;
}

/**
 * 機關定期擷取設定表單
 * @param
 */
function ndcop_dataset_api_exchang_org_harvest_setting_form( $form, &$form_state, $org_term ){

	$form = array();
	$form_state['ndcop_dataset_api']['org_term'] = $org_term;

	if( isset($org_term->field_harvest_begin[LANGUAGE_NONE]) ){
		list($hour, $min) = explode(":", $org_term->field_harvest_begin[LANGUAGE_NONE][0]['value']);
	}
	else{
		$hour = '0';
		$min = '00';
	}

	$form['harvest_api_sru'] = array(
		'#type' => 'textfield',
		'#title' => t('資料集API網址'),
		'#default_value' => $org_term->field_harvest_api_sru[LANGUAGE_NONE][0]['value'],
		'#size' => 60,
		'#maxlength' => 128,
		'#required' => TRUE,
		'#attributes' => array('placeholder'=>'http://data.nat.gov.tw/api/v1'),
		'#label_attributes' => array('class' => array('title') ),
	);

	$form['harvest_freq'] = array(
       '#type' => 'select',
       '#title' => t('定期擷取頻率'),
       '#options' => array(
          24 => t('每日'),
       ),
       '#default_value' => $org_term->field_harvest_freq[LANGUAGE_NONE][0]['value'],
       '#label_attributes' => array('class' => array('title') ),
   	);

	$form['harvest_begin_hour']	= array(
	    '#type' => 'container',
	    '#attributes' => array('class' => array('container-inline')),
  	);
	$form['harvest_begin_hour']['harvest_begin_hour'] = array(
       '#type' => 'select',
       '#title' => t('定期擷取起始時間'),
       '#options' => ndcop_dataset_api_generate_retrieve_begin_time(),
       '#prefix' => '<div id="retrieve_begin_hour">',
       '#suffix' => '</div>時',
       '#default_value' => $hour,
       '#label_attributes' => array('class' => array('title') ),
    );

    $form['harvest_begin_hour']['harvest_begin_min'] = array(
       '#type' => 'select',
       '#options' => array('00'=>'00', '30'=>'30',),
       '#suffix' => '分',
       '#default_value' =>  $min,
    );
    $form['harvest_success_time'] = array(
        '#title' => t('上次擷取完成時間'),
        '#type' => 'item',
        '#markup' => isset($org_term->field_harvest_success_time[LANGUAGE_NONE]) ? date('Y-m-d',$org_term->field_harvest_success_time[LANGUAGE_NONE][0]['value']) : t('尚未開始擷取') ,
        '#prefix' => '<div id="success_time">',
        '#suffix' => '</div>',
        '#label_attributes' => array('class' => array('title') ),
    );

    $form['submit'] = array(
	    '#type'=>'submit',
    	'#value' => t('儲存設定'),
  	);

  	$form['harvest_test'] = array(
	    '#type'=>'submit',
    	'#value' => t('測試擷取'),
  	);
  	$uid = ( isset( $_SESSION['account']['uid'] ) ) ? $_SESSION['account']['uid'] : $user->uid ;
  	$form['back'] = array(
	    '#markup'=> '<input type=\'button\' class=\'button-user\' value=\'回到後台管理者\' onclick=\'location.href="http://'.$_SERVER['HTTP_HOST'].'/admin_back/'. $uid .'/apiexchange/config"\'/>'
  	);

	return $form;
}

/**
 * 機關定期擷取設定表單 Submit Callback
 * @param
 */
function ndcop_dataset_api_exchang_org_harvest_setting_form_submit( $form, &$form_state ){

	module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.harvest');
	global $user;
	// Form Button's value
  	$op = $form_state['values']['op'];
  	$org_term = $form_state['ndcop_dataset_api']['org_term'];
	switch ($op) {
    	case t('儲存設定'):
	    	drupal_set_message($op);
	    	// Harvest資料集API網址
	    	$org_term->field_harvest_api_sru[LANGUAGE_NONE][0]['value'] = $form_state['values']['harvest_api_sru'];
	    	// Harvest定期擷取頻率
	    	$org_term->field_harvest_freq[LANGUAGE_NONE][0]['value'] = $form_state['values']['harvest_freq'];
	    	// Harvest定期擷取起始時間
	    	$org_term->field_harvest_begin[LANGUAGE_NONE][0]['value'] = $form_state['values']['harvest_begin_hour'].':'.$form_state['values']['harvest_begin_min'];
	    	// Harvest排程設定更新時間
	    	$org_term->field_harvest_setting_updated[LANGUAGE_NONE][0]['value'] = time();
    		taxonomy_term_save($org_term);

	    break;
	    case t('測試擷取'):
	    	drupal_set_message($op);

	    	$retrieve_api_url = $form_state['values']['harvest_api_sru'].'/rest/dataset';
            $param = array(
                'modified' => '2016-08-17',
                'oid' => '2.16.886.101.20003.20031',
                'publish_api_key' => '4360ed6d-9f13-6d08-59a3-0657e3b32af5'
                );
    		$test_result = ndcop_dataset_api_test_harvest_api_sru($retrieve_api_url);
    		drupal_set_message(print_r($test_result,true));
    		if ($test_result) {
    			drupal_set_message('測試擷取 '.$retrieve_api_url.' 成功');
    		}
    		else{
    			drupal_set_message('測試擷取 '.$retrieve_api_url.' 失敗', 'error');
    		}
	    break;
    }
}

/**
 * 機關Publish APIKEY和IP設定表單
 * @param
 */
function ndcop_dataset_api_exchang_org_publish_setting_form( $form, &$form_state, $org_term ){

	$form = array();
	$form_state['ndcop_dataset_api']['org_term'] = $org_term;

	if( !empty($org_term->field_publisher_api_key['und'][0]['value']) ){
		$apikey = $org_term->field_publisher_api_key['und'][0]['value'];
	}
	else{
		$apikey = ndcop_api_get_guid();
	}

	$form['apikey_ip_region'] = array(
	    '#type' => 'fieldset',
	    '#title' => t('設定機關IP與API Key頁面'),
  	);

  	$form['apikey_ip_region']['container']	= array(
	    '#type' => 'container',
	    '#attributes' => array('class' => array('w1')),
  	);
	$form['apikey_ip_region']['container']['publisher_ip'] = array(
		'#type' => 'textfield',
		'#title' => t('發佈機關IP'),
		'#description' => t('若有多個IP，請使用逗點(,)分隔'),
		'#default_value' => $org_term->field_publisher_ip['und'][0]['value'],
		'#size' => 60,
		'#maxlength' => 512,
		'#required' => TRUE,
		'#attributes' => array('placeholder'=>'請填入啟動publish的IP')
	);

	$form['apikey_ip_region']['container']['show_apikey'] = array(
        '#title' => t('API KEY'),
        '#type' => 'item',
        '#markup' => $apikey,
        '#prefix' => '<div id="apikey">',
        '#suffix' => '</div>',
    );

	$form['apikey_ip_region']['container']['send_publisher_mail'] = array(
	    '#type' => 'radios',
	    '#title' => t('是否將APIKEY設定通知發佈機關'),
	    '#default_value' => 'n',
	    '#options' => array('y' => t('是'), 'n' => t('否')),
  	);
	$form['apikey_ip_region']['container']['publisher_mail'] = array(
		'#type' => 'textfield',
		'#title' => t('通知發佈機關電子郵件'),
		'#description' => t('若有多個電子郵件，請使用逗點(,)分隔'),
		'#default_value' => $org_term->field_publisher_mail['und'][0]['value'],
		'#size' => 60,
		'#maxlength' => 128,
		'#attributes' => array('placeholder'=>'請填入通知發佈機關的電子郵件'),
		'#states' => array(
		  'visible' => array(
		    ':input[name="send_publisher_mail"]' => array('value' => 'y'),
		),
),
	);

	$form['apikey_ip_region']['container']['hidden_apikey'] = array(
		'#type' => 'hidden',
		'#default_value' => isset($form_state['values']['hidden_apikey']) ? $form_state['values']['hidden_apikey'] : $apikey ,
	);

	$form['tid'] = array(
		'#type' => 'hidden',
		'#value' => $org_term->tid
	);

	$form['apikey_ip_region']['container']['submit'] = array(
        '#type' => 'submit',
        '#value' => '儲存',
    );

	$uid = ( isset( $_SESSION['account']['uid'] ) ) ? $_SESSION['account']['uid'] : $user->uid ;
  	$form['apikey_ip_region']['container']['back'] = array(
	    '#markup'=> '<input type=\'button\' class=\'button-user\' value=\'回到後台管理者\' onclick=\'location.href="http://'.$_SERVER['HTTP_HOST'].'/admin_back/'. $uid .'/apiexchange/config"\'/>'
  	);

	return $form;
}

/**
 * 機關Publish APIKEY和IP設定表單 Submit Callback
 * @param
 */
function ndcop_dataset_api_exchang_org_publish_setting_form_submit( $form, &$form_state ){
	global $user;
	// Form Button's value
  	$op = $form_state['values']['op'];
  	$org_term = $form_state['ndcop_dataset_api']['org_term'];

  	switch ($op) {
    	case t('儲存'):
	    	$org_term->field_publisher_api_key[LANGUAGE_NONE][0]['value'] = $form_state['values']['hidden_apikey'];
	    	// 發佈機關IP
	    	$org_term->field_publisher_ip[LANGUAGE_NONE][0]['value'] = $form_state['values']['publisher_ip'];
	    	$org_term->field_publisher_mail[LANGUAGE_NONE][0]['value'] = $form_state['values']['publisher_mail'];
    		taxonomy_term_save($org_term);

    		// 通知發佈機關APIKEY設定
    		if( $form_state['values']['send_publisher_mail'] == 'y' and !empty($form_state['values']['publisher_mail']) ){

    			try{
    				// 產生加密APIKEY檔案
    				$text = 'APIKEY:'. $form_state['values']['hidden_apikey']. PHP_EOL . 'IP: ' . $form_state['values']['publisher_ip'] ;
					$filename = 'publish_apikey_'.time().'.txt';   // Filename
					$file_path = 'temporary://' . $filename;
					$file = file_unmanaged_save_data($text, $file_path);
					$password = $form_state['values']['publisher_ip'];
					$outfilename = 'publish_apikey_'.time().'.zip';
					$outfile_path = 'temporary://' . $outfilename;
					$tmp_dir = file_directory_temp().'/';
					echo system('zip -P '. $password .' '.$tmp_dir.$outfilename. ' '. $tmp_dir.$filename);

					$message = drupal_mail('ndcop_dataset_api', 'apikey_info', 'opendata@ndc.gov.tw', LANGUAGE_NONE, array('publisher_apikey'=>$form_state['values']['hidden_apikey'], 'publisher_ip' => $form_state['values']['publisher_ip'], 'publisher_mail' => $form_state['values']['publisher_mail'], 'apikeyfilepath' => $tmp_dir.$outfilename, 'apikeyfilename'=>$outfilename, 'unzippass'=>$password), 'opendata@ndc.gov.tw', TRUE);
					if($message){
						// 發信完成，刪除相關檔案
					    file_unmanaged_delete($tmp_dir.$filename);
					    file_unmanaged_delete($tmp_dir.$outfilename);
					    drupal_set_message('設定完成。');
					}
				}
				catch(Exception $e){
					drupal_set_message('Publish APIKEY申請回覆信寄送郵件失敗'.$e->getMessage(), 'error');
					watchdog('ndcop_dataset_api','%data',
			 			array('%data' => 'Publish APIKEY申請回覆信寄送郵件失敗'.$e->getMessage(),'%action'=>$action, WATCHDOG_DEBUG));
					return null;
				}
    		}
    		else{
    			drupal_set_message('設定完成。');
    		}

	    break;
    }
}

/**
 * 跨平臺介接紀錄查詢畫面過濾表單
 * @param
 */
function ndcop_dataset_api_exchang_log_filter_form( $form, &$form_state, $backend_role = 'admin_back' ){
	$form = array();

	$form['filters'] = array(
	    '#type' => 'fieldset',
	    '#title' => t(''),
		'#attributes' => array('class' => array('areaBlue')),
	);

	if( $backend_role == 'admin_back' ){
		$form['filters']['log_api_type'] = array(
	       '#type' => 'select',
	       '#title' => t('介接'),
	       '#options' => array('All'=>'所有','Publish'=>'Publish', 'Harvest'=>'Harvest',),
	       '#default_value' =>  isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_type'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_type'] : 'All',
	    );
	}
	$form['filters']['log_api_result'] = array(
       '#type' => 'select',
       '#title' => t('執行結果'),
       '#options' => array(-1=>'全部結果',1=>'成功', 0=>'失敗',),
       '#default_value' =>  isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'] : -1,
    );

	$form['filters']['filter_start'] = array(
   		'#title' => t('開始日期'),
        '#type' => 'date_popup',
		'#required' => TRUE,
		'#date_format' => 'Y/m/d',
		'#date_label_position' => 'none',
		'#date_year_range' => '-20:+0',
		'#default_value' => isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'] : date("Y-m-d", strtotime('-1 week')),
		'#weight' =>'8'
   	);

	$form['filters']['filter_end'] = array(
   		'#title' => t('結束日期'),
        '#type' => 'date_popup',
		'#date_label_position' => 'none',
		'#required' => TRUE,
		'#date_format' => 'Y/m/d',
		'#date_year_range' => '-20:+0',
		'#default_value' => isset($_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'])  ? $_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'] : date("Y-m-d"),
		'#weight' =>'9'
   	);

	$form['filters']['submit'] = array(
	    '#type'=>'submit',
    	'#value' => t('查詢'),
    	'#weight' =>'10',
  	);

  	$form['filters']['reset'] = array(
	    '#type'=>'submit',
    	'#value' => t('重設'),
    	'#weight' =>'11',
  	);

	return $form;
}

function ndcop_dataset_api_exchang_log_filter_form_submit($form, &$form_state){

	$op = $form_state['values']['op'];
	global $user;

	switch ($op) {
		case t('查詢'):
			if(isset($form_state['values']['log_api_type'])){
				$_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_type'] = $form_state['values']['log_api_type'] ;
			}
			$_SESSION['ndcop_dataset_api_exchang_log_filter']['log_api_result'] = $form_state['values']['log_api_result'] ;
			$_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_start'] = $form_state['values']['filter_start'] ;
			$_SESSION['ndcop_dataset_api_exchang_log_filter']['filter_end'] = $form_state['values']['filter_end'] ;
			break;
    	case t('重設'):
    		unset($_SESSION['ndcop_dataset_api_exchang_log_filter']);
			break;
    }
	return;
}

function ndcop_dataset_api_org_tid_name_mapping( $org_tids ){

	$tid_name_mapping = array();
	$tmp_tid_name_mapping = array();

	foreach ($org_tids as $org_tid) {
		$org = taxonomy_term_load( $org_tid );
		$tmp_tid_name_mapping[ $org_tid ] = $org->name;
	}

	// 先放置全部機關
	$all_orgs = ndcop_dataset_api_get_all_orgs_options( $org_tids );
	$tid_name_mapping[implode(',', $all_orgs)] = t('全部機關');
	$mppping_result = $tid_name_mapping + $tmp_tid_name_mapping;
	return $mppping_result;
}

/**
 * 全部機關tid陣列
 * @param $org_tids
 */
function ndcop_dataset_api_get_all_orgs_options( $org_tids ){

	$all_orgs = array();

	foreach ($org_tids as $org_tid) {
		$org = taxonomy_term_load( $org_tid );
		$all_orgs[] = $org_tid;
	}
	return $all_orgs;
}

/**
 * 產生定期擷取起始時間
 * @see ndcop_dataset_api_exchang_org_harvest_setting_form
 */
function ndcop_dataset_api_generate_retrieve_begin_time(){
	$retrieve_begin_time = array();
	for ($hour = 2; $hour < 24; $hour++) {
		$retrieve_begin_time[$hour] = $hour;
	}
	return $retrieve_begin_time;

}

/**
 * 取得API介接紀錄
 * @see
 * @param $header
 * @param $filter_condition
 */
function ndcop_dataset_api_get_exchang_log($header, $filter_condition, $publisherIpAddr = ''){

	$query = db_select('ndcop_dataset_api_log','ndcop_dataset_api_log')
    ->fields('ndcop_dataset_api_log');

	// 使用者填入篩選條件
    if( !empty( $filter_condition['log_api_type'] ) ){
      if( $filter_condition['log_api_type'] == 'Publish' ){
         $query->condition('ndcop_dataset_api_log.request_method', array('POST', 'PUT', 'DELETE'), 'IN');
     }
     elseif( $filter_condition['log_api_type'] == 'Harvest' ){
         $query->condition('ndcop_dataset_api_log.request_method','%'.db_like($filter_condition['log_api_type']) . '%', 'LIKE');
     }
 }
 if(!empty($publisherIpAddr)){
    $query->condition('ndcop_dataset_api_log.request_ip',$publisherIpAddr,'IN');
}
if( in_array($filter_condition['log_api_result'], array(0, 1)) ){
  $query->condition('ndcop_dataset_api_log.success', $filter_condition['log_api_result'] );
}

if( !empty($filter_condition['filter_start']) and !empty($filter_condition['filter_end']) ){
		// 一天內，若過濾條件為2015-04-22，則區間為2015-04-22至2015-04-23
  $start_from = $filter_condition['filter_start'];
  $start_to = $filter_condition['filter_end'];
  $query->condition('ndcop_dataset_api_log.req_time', array($start_from, $start_to), 'BETWEEN' );
}

if( isset($filter_condition['tablesort']) && $filter_condition['tablesort'] == TRUE ){
  $query->extend('TableSort')->orderByHeader($header);
}
if( isset($filter_condition['pager']) && $filter_condition['pager'] == TRUE ){
  $query = $query->extend('PagerDefault')->limit(50);
}
$results = $query->execute();

return $results;
}
