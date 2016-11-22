<?php

/**
 * @file
 * 負責處理資料集詮釋資料API之排程.
 * TODO:
 */

/**
 *	NDC OPENDATA API定期擷取排程(Harvest)
 *  @param
 */
function ndcop_dataset_api_harvest_cron(){

	watchdog('ndcop_dataset_api_harvest','%data',
		array('%data' => date('Y-m-d H:i:s').' NDC OPENDATA API定期擷取排程(Harvest)開始!' , WATCHDOG_DEBUG));
	// 排程開始時間
	$cron_start_time = time();

	// 有設定Harvest的機關
	$harvest_org_configs = ndcop_dataset_api_harvest_get_orgs();

	foreach ($harvest_org_configs as $harvest_org_config ) {

		// 符合設定值條件，可以啟動Harvest
		if( ndcop_dataset_api_check_harvest( $harvest_org_config, $cron_start_time ) == 0 ){
		if(true){
			watchdog('ndcop_dataset_api_harvest','%name, %data, Config = %config',
				array('%data' => '符合設定值條件，啟動Harvest!','%name'=>$harvest_org_config->name, '%config'=>print_r($harvest_org_config, true), WATCHDOG_DEBUG));

			$param = array(
				'modified' => $harvest_org_config->success_time,
				'oid' => $harvest_org_config->oid,
				'publish_api_key' => $harvest_org_config->apikey,
				);

			module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.harvest');
			$harvest_result = ndcop_dataset_api_harvest_process( $harvest_org_config->harvest_api_sru, $param );

			if($harvest_result == TRUE){
				watchdog('ndcop_dataset_api_harvest','%name, %data', array('%data' => 'Harvest完成!','%name'=>$harvest_org_config->name, '%config'=>print_r($harvest_org_config, true), WATCHDOG_DEBUG));

	    		// 更新定期擷取完成時間
				$org_term = taxonomy_term_load($harvest_org_config->tid);
				$org_term->field_harvest_success_time[LANGUAGE_NONE][0]['value'] = time();
				taxonomy_term_save($org_term);

			}
		}
		else{

			watchdog('ndcop_dataset_api_harvest','%name, %data, OID = %oid, Config = %config',
				array('%data' => '不符合設定值條件，不啟動Harvest!','%name'=>$harvest_org_config->name, '%config'=>print_r($harvest_org_config, true), WATCHDOG_DEBUG));
		}
	}
}
}

/**
 *	NDC OPENDATA API定期擷取排程(Harvest) 30分執行
 *  @param
 */
function ndcop_dataset_api_harvest_halfclock_cron(){
	ndcop_dataset_api_harvest_cron();
}

/**
 *	取得有設定Harvest的機關
 *  @return $orgs: 機關設定值
 */
function ndcop_dataset_api_harvest_get_orgs(){

	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'taxonomy_term')
	->entityCondition('bundle', array('org'))
	->fieldCondition('field_harvest_api_sru', 'value', 'NULL', '!=');

	$result = $query->execute();
	$tids = array_keys($result['taxonomy_term']);

	$orgs = array();
	foreach ($tids as $tid) {

		$org_term_param = new stdClass();

		$org_term = taxonomy_term_load($tid);

		$org_term_param->name = $org_term->name;
		$org_term_param->tid = $tid;

		$org_term_param->harvest_api_sru = $org_term->field_harvest_api_sru[LANGUAGE_NONE][0]['value'];

		// 排程設定更新時間
		$org_term_param->updated_time = $org_term->field_harvest_setting_updated[LANGUAGE_NONE][0]['value'];
		// 定期擷取起始時間
		$org_term_param->retrieve_begin = $org_term->field_harvest_begin[LANGUAGE_NONE][0]['value'];
		// 定期擷取頻率
		$org_term_param->retrieve_freq = $org_term->field_harvest_freq[LANGUAGE_NONE][0]['value'];

		// OID
		$org_term_param->oid = $org_term->field_orgoid[LANGUAGE_NONE][0]['value'];

		// Publish APIKEY
		if( isset($org_term->field_publisher_api_key[LANGUAGE_NONE]) ){
			$org_term_param->apikey = $org_term->field_publisher_api_key[LANGUAGE_NONE][0]['value'];
		}
		else{
			// 若尚未設定Publish APIKEY
			// 產生Publish APIKEY
			module_load_include('inc', 'ndcop_api', 'ndcop_api.common');
			$uuid = ndcop_api_get_guid();
			$org_term_param->apikey = $uuid;
			$org_term->field_publisher_api_key[LANGUAGE_NONE][0]['value'] = $uuid;
			taxonomy_term_save( $org_term );
		}

		// 定期擷取完成時間
		if( isset($org_term->field_harvest_success_time[LANGUAGE_NONE]) ){
			$org_term_param->success_time = date('Y-m-d', $org_term->field_harvest_success_time[LANGUAGE_NONE][0]['value'] );
		}
		else{
			// 第一次發動Harvest
			$org_term_param->success_time = '1900-01-01';
		}

		$orgs[] = $org_term_param;

	}

	return $orgs;
}

/**
 *	判斷是否可以啟動NDC OPENDATA API定期擷取排程
 *  @param $result : 定期擷取排程設定值
 *  @param $cron_execution_time : 排程啟動時間
 *  @return $cron_start: 0(表示可以啟動) ; 其他值(表示沒有滿足設定值)
 */
function ndcop_dataset_api_check_harvest( $result, $cron_execution_time ){

	// 定期擷取排程設定完成時間
	$setting_updated_date = date('Y-m-d', $result->updated_time);

	// 排程啟動時間
	$cron_execution_date = date('Y-m-d', $cron_execution_time);
	$cron_execution_hour = intval( date('G', $cron_execution_time) );
	$cron_execution_min = intval( date('i', $cron_execution_time) ) / 60;
	$cron_execution_min_str = number_format($cron_execution_min, 2, '.', '');
	$cron_execution_min_for_execution_delay = floatval( substr($cron_execution_min_str, 0, 3) );

	list($begin_hour, $begin_min) = explode(":", $result->retrieve_begin);
	$begin_hour = intval($begin_hour);
	$begin_min = intval($begin_min)/60;

	// 定期擷取排程設定完成時間和排程啟動時間相差的天數
	$cron_execution_time_obj = date_create($cron_execution_date);
	$setting_updated_time_obj = date_create($setting_updated_date);
	$diff = date_diff($setting_updated_time_obj, $cron_execution_time_obj);
	$diff_days = $diff->days;

	$parameters = '$diff_days: '.$diff_days.' $cron_execution_hour+$cron_execution_min_for_execution_delay: '.($cron_execution_hour+$cron_execution_min_for_execution_delay).' $begin_hour+$begin_min: '.($begin_hour+$begin_min).' $result->retrieve_freq: '.$result->retrieve_freq;

	// 計算設定值是否符合啟動條件
	$cron_start = ( ($diff_days * 240) + ( ( ($cron_execution_hour+$cron_execution_min_for_execution_delay) * 10 ) - ( ($begin_hour+$begin_min) * 10 ) ) ) % ( $result->retrieve_freq * 10 );
	watchdog('ndcop_dataset_api_harvest','ndcop_dataset_api_harvest: %cron_jobs_time',
		array('%cron_jobs_time' => $parameters.' $cron_start: '.$cron_start, WATCHDOG_DEBUG));
	return $cron_start;
}
