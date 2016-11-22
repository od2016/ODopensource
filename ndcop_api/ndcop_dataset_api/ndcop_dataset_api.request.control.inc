<?php

/**
 * @file
 * 負責處理Restful Request(GET, POST, PUT, DELETE)
 * TODO:
 */

// 載入dataset(GET), dataset異動(POST, PUT, DELETE)
module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.dataset');
module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.dataset.modify');

 /**
 *  取得資料集編號清單(GET)或新增詮釋資料(POST) page callback
 *  @see hook_menu
 */
 function ndcop_dataset_api_control_dataset(){

 	switch ($_SERVER['REQUEST_METHOD']) {
		//取得資料集清單
 		case 'GET':
 		try{
	      		// Returns a list of dataset.
 			ndcop_dataset_api_dataset_list();
 		}
 		catch(Exception $e){
 			echo ($e->getMessage());
 			drupal_set_message($e->getMessage(), 'error');
 			return null;
 		}
 		break;
	    //新增詮釋資料
 		case 'POST':
 		$validation = array();
 		$json_format_validate = array();
 		$publisher_validation = array();
 		$api_in_data_validate = array();

	        // 判斷API Key和 IP
 		$validation = ndcop_api_validate_apikey_ip_process();

 		if($validation['pass']){
 			$post_data_json_str = file_get_contents('php://input');
 			$ndc_opdata_format_obj = json_decode($post_data_json_str );
		        // LOG進來的JSON DATA
 			watchdog('ndcop_dataset_api','SRU/rest/dataset(POST Method) Input json data: %json', array('%json' => $post_data_json_str, WATCHDOG_DEBUG));

		        // 判斷外部json data格式是否有誤
 			$json_format_validate = ndcop_api_validate_json_format($ndc_opdata_format_obj);

 			if( $json_format_validate['pass'] ){
			        // 判斷外部json data中的publisher機關和API KEY的機關是否符合
 				$publisher_validation = ndcop_api_validate_publisher_org( $ndc_opdata_format_obj->publisher, $ndc_opdata_format_obj->publisherOID );

 				if( $publisher_validation['pass'] ){
			        	// 已通過APIKey和IP驗證，確保$headers['Authorization']正確
 					$headers = getallheaders();
 					$authorization_api_key =  $headers['Authorization'];
			        	// 驗證API IN的JSON DATA
 					$api_in_data_validate = ndcop_api_validate_api_in_data_process('create', $authorization_api_key, $post_data_json_str);

                    if( $api_in_data_validate['pass'] ){
			        		// 已通過APIKey和IP驗證，確保$headers['Authorization']正確
 						$headers = getallheaders();
 						$authorization_api_key =  $headers['Authorization'];
 						$apikey_info = ndcop_api_check_apikey_info($authorization_api_key);
				        	// 新增詮釋資料
 						$ndcop_api_id = ndcop_dataset_api_create_dataset($post_data_json_str, $apikey_info);
 						ndcop_api_show_result_msg($ndcop_api_id, json_encode(''));

 					}
 					else{
 						ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($api_in_data_validate['error']));
 					}
 				}
 				else{

 					ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($publisher_validation['error']));
 				}
 			}
 			else{
 				ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($json_format_validate['error']));
 			}
	    	} //判斷API Key和 IP End
	    	else{
	    		ndcop_api_show_result_msg('', json_encode($validation['error']));
	    	}
	    	break;
	    	default:
        # code...
	    	break;
	    }
	}

 /**
 *  取得資料集詮釋資料(GET)、修改資料集詮釋資料(PUT)、刪除詮釋資料(DELETE) page callback
 *  @see hook_menu
 */
 function ndcop_dataset_api_control_dataset_identifier( $identifier ){

	// Sanitize $tag
 	$identifier = check_plain($identifier);

 	switch ($_SERVER['REQUEST_METHOD']) {
 		case 'GET':
 		ndcop_dataset_api_dataset_show($identifier);
 		break;
 		case 'PUT':
 		$validation = array();
 		$json_format_validate = array();
 		$publisher_validation = array();
 		$api_in_data_validate = array();

	        // 判斷API Key和 IP
 		$validation = ndcop_api_validate_apikey_ip_process();

 		if($validation['pass']){

 			$post_data_json_str = file_get_contents('php://input');
 			$ndc_opdata_format_obj = json_decode($post_data_json_str );
		        // LOG進來的JSON DATA
 			watchdog('ndcop_dataset_api','SRU/rest/dataset/{identifier}(PUT Method) Input json data: %json', array('%json' => $post_data_json_str, WATCHDOG_DEBUG));

		        // 判斷外部json data格式是否有誤
 			$json_format_validate = ndcop_api_validate_json_format($ndc_opdata_format_obj);

 			if( $json_format_validate['pass'] ){
 			// 判斷外部json data中的publisher機關和API KEY的機關是否符合
 				$publisher_validation = ndcop_api_validate_publisher_org( $ndc_opdata_format_obj->publisher, $ndc_opdata_format_obj->publisherOID );

 				if( $publisher_validation['pass'] ){
			        	// 已通過APIKey和IP驗證，確保$headers['Authorization']正確
 					$headers = getallheaders();
 					$authorization_api_key =  $headers['Authorization'];
			        	// 驗證API IN的JSON DATA
 					$api_in_data_validate = ndcop_api_validate_api_in_data_process('update', $authorization_api_key, $post_data_json_str, $identifier);

 					if( $api_in_data_validate['pass'] ){
 						$update_dataset_nid = $api_in_data_validate['dataset_nid'];
 						// 已通過APIKey和IP驗證，確保$headers['Authorization']正確
 						$headers = getallheaders();
 						$authorization_api_key =  $headers['Authorization'];
 						$apikey_info = ndcop_api_check_apikey_info($authorization_api_key);
				    // 修改詮釋資料
 						$ndcop_api_id = ndcop_dataset_api_update_dataset($post_data_json_str, $apikey_info, $update_dataset_nid);
 						ndcop_api_show_result_msg($ndcop_api_id, json_encode(''), $post_data_json_str);
 					}
 					else{
						ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($api_in_data_validate['error']), $post_data_json_str );
 					}
 			} // 判斷機關是否相符 End
 			else{
 				ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($publisher_validation['error']), $post_data_json_str );
 			}
 		} // 判斷JSON Format End
 		else{
 			ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($json_format_validate['error']), $post_data_json_str );
 		}
 	} // 判斷API Key和 IP End
 	else{
 		ndcop_api_show_result_msg('', json_encode($validation['error']) );
 	}
 	break;
 	case 'DELETE':
 	$validation = array(); // API Key和 IP驗證
 	$check_dataset_exist = FALSE; // 資料集是否存在
			// 判斷API Key和 IP
 	$validation = ndcop_api_validate_apikey_ip_process();

 	if($validation['pass']){
 	// 判斷API IN的資料集是否存在於平台上
	// 在確認資料正確性流程前，已通過APIKey和IP驗證，確保$headers['Authorization']正確
 		$headers = getallheaders();
 		$apikey_info = ndcop_api_check_apikey_info($headers['Authorization']);
 		$check_identifier = $identifier;
 		$ndcop_id = ndcop_api_get_check_apiin_identifier($check_identifier, $apikey_info);
 		$check_dataset_exist = ( !empty($ndcop_id) ) ? TRUE : FALSE ;

 		if(	$check_dataset_exist ){
 			$dataset = node_load( $ndcop_id );
 			$dataset->status = 0;
 			$dataset->field_dataset_status[LANGUAGE_NONE][0]['value'] = '9';
 			$dataset->field_end_date[LANGUAGE_NONE][0]['value'] = time();
 			// 透過API刪除
 			$dataset->field_dataset_modify_method[LANGUAGE_NONE][0]['value'] = '1';
 			node_save($dataset);
 			ndcop_api_show_result_msg( $check_identifier, json_encode(''), '{"identifier":"'.$check_identifier.'"}');
 		}
 		else{
 			$error['error']['name'][] = '欲刪除的資料集不存在';
 			$error['error']['__type'] = 'ER0060:欲刪除的資料集不存在';
 			ndcop_api_show_result_msg( $check_identifier, json_encode($error), '{"identifier":"'.$check_identifier.'"}');
 		}
 	} // 判斷API Key和 IP End
 	else{
 		ndcop_api_show_result_msg('', json_encode($validation['error']) );
 	}
 	break;
 	default:
 	header($_SERVER["SERVER_PROTOCOL"]." 404 Not Found");
 	drupal_exit();
 	break;
 }
}
function ndcop_dataset_api_control_count_statistic($oid)
{
    switch ($_SERVER['REQUEST_METHOD']) {
        case 'GET':
        $rst = ndcop_dataset_api_get_node_count_by_oid($oid);
        break;

        default:
            header($_SERVER["SERVER_PROTOCOL"]." 405 Method Not Allowed");
            drupal_exit();
        break;
    }
}

function ndcop_dataset_api_control_dataset_identifier_list($oid)
{
    switch ($_SERVER['REQUEST_METHOD']) {
        case 'GET':
        $rst = ndcop_dataset_api_get_dataset_identifier_by_oid($oid);
        break;

        default:
            header($_SERVER["SERVER_PROTOCOL"]." 405 Method Not Allowed");
            drupal_exit();
        break;
    }
}
