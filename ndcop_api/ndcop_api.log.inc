<?php

/**
 * @file
 * 負責處理API紀錄LOG的部分.
 * TODO: 
 */

/**
 * 針對資料存取應用程式介面(API)功能進行紀錄
 * @see 
 * @param $action
 * @param $ip
 * @param $access_url
 * @param $json_data
 */
function ndcop_api_dataset_log( $request_method, $request_ip, $request_uri, $json_data, $success, $err_type, $result , $req_time){

	try{
		$log_data = array(
			  'request_method' => $request_method,
			  'request_ip' => $request_ip,
			  'request_uri' =>  $request_uri,
			  'api_in_data' => $json_data,
			  'success' =>  $success,
			  'err_type' => $err_type,  
			  'result' => $result,
			  'req_time' => date('Y-m-d H:i:s' ,$req_time)
			);
		db_insert('ndcop_dataset_api_log')->fields($log_data)->execute();

	}catch(Exception $e){
		watchdog('ndcop_dataset_api', '新增詮釋資料API紀錄出現錯誤, Log資料為 @logdata, Message為@msg', array('@logdata' => print_r($log_data,true), '@msg'=>$e->getMessage() ) );
		return null;
	}
}