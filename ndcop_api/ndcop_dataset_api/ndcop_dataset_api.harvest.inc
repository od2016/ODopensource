<?php

/**
 * @file
 * 資料集詮釋資料API定期擷取相關操作.
 * TODO:
 */

/**
 * Harvest擷取流程
 * 1. 取得擷取機關平台上詮釋資料最後更新時間為上一次擷取成功時間到現在的詮釋資料，進行更新或新增
 * 2. 比對其他開放平臺所有清單(不帶參數)和NDC政府資料開放平臺清單。將NDC政府資料開放平臺清單和其他開放平臺所有清單同步成一致
 * @see
 * @param $harvest_api_sru 資料集API網址
 * @param $param Array( modified 上一次擷取成功的時間, oid OID )
 * @return
 */

function ndcop_dataset_api_harvest_process( $harvest_api_sru, $param ){

    watchdog('ndcop_dataset_api','%data, OID = %oid',
        array('%data' => '資料集詮釋資料API定期擷取開始!','%oid'=>$param['oid'], WATCHDOG_DEBUG));

    $name = 'harvest_process';
    timer_start($name);

    try{
        // 取得NDC政府資料平臺上dataset 的API IN identifier集合.
        module_load_include('inc', 'ndcop_api', 'ndcop_api.common');
        $apiin_id_set = ndcop_api_get_apiin_dataset_id( $param['oid'] );


        /*取得擷取機關平台上詮釋資料最後更新時間為上一次擷取成功時間到現在的詮釋資料，進行更新或新增*/
        /*----------------------------------------------------------------*/
        if( isset( $param['modified'] ) ){
            // 詮釋資料最後更新時間大於或等於上一次擷取成功的時間
            $harvest_api_accesspoint = $harvest_api_sru.'/rest/dataset?modified='.$param['modified'];
        }
        // 判斷是否為政府資料開放平臺的SRU，加上參數oid
        if( isset( $param['oid'] ) and ndcop_dataset_api_check_ndcsite_sru( $harvest_api_sru ) ){
            $harvest_api_accesspoint .= ('&oid=' . $param['oid'] );
        }
        // 取得詮釋資料最後更新時間為上一次擷取成功時間到現在的詮釋資料
        $retrieve_org_datalist_modified = ndcop_dataset_api_retrieve_datalist( $harvest_api_accesspoint );

        watchdog('ndcop_dataset_api','Harvest access point: %access ; retrieve_org_datalist_modified: %data',
           array('%access'=> $harvest_api_accesspoint,'%data' => print_r($retrieve_org_datalist_modified,true), WATCHDOG_DEBUG));

        // 有取得回傳值
        if( !empty($retrieve_org_datalist_modified) ){

            // 取得異動資料集序號清單(根據不同平台可能會有不同的取法)
            if( ndcop_dataset_api_check_ndcsite_sru( $harvest_api_sru ) ){
                $modified_id_list = $retrieve_org_datalist_modified->result;
            }
            else{
                $modified_id_list = $retrieve_org_datalist_modified;
            }

            foreach($modified_id_list as $key => $identifier ){
            	// 判斷詮釋資料是否已存在
            	$dataset_exist = in_array($identifier, $apiin_id_set);
                // 取得詮釋資料(必須為NDC規範格式的JSONDATA)
                // 若Harvest進來的JSON不為NDC規範格式的JSONDATA，必須先轉換成NDC規範格式的JSONDATA
                $ndcopdata_api_format_json_data_string = ndcop_dataset_api_retrieve_data( $harvest_api_sru, $identifier );

                if( !$dataset_exist ){
            		// 新增詮釋資料
                  watchdog('ndcop_dataset_api','Harvest新增詮釋資料開始, id=%id', array('%id' => $identifier, WATCHDOG_DEBUG));
                  timer_start($identifier);

                  $publisher_apikey = $param['publish_api_key'];
                    // 呼叫資料集異動(Publish)API新增
                  $result = ndcop_dataset_api_harvest_create_dataset( $ndcopdata_api_format_json_data_string, $publisher_apikey );

                  timer_stop($identifier);

                  ndcop_api_log_harvest_result('Harvest_Create', $harvest_api_sru, $harvest_api_sru.'/rest/dataset/'. $identifier , $result['id'], $result['ndc_harvest_result'] , $ndcopdata_api_format_json_data_string );

                  watchdog('ndcop_dataset_api','Harvest新增詮釋資料結束, id=%id, time=%time, result=%result', array('%id' => $identifier,'%time' => (timer_read($identifier)/1000).'s','%result'=>$result, WATCHDOG_DEBUG));
              }
              else{
            		// 更新詮釋資料
                  watchdog('ndcop_dataset_api','Harvest更新詮釋資料開始, id=%id', array('%id' => $identifier, WATCHDOG_DEBUG));
                  timer_start($identifier);

                  $publisher_apikey = $param['publish_api_key'];
                    // 呼叫資料集異動(Publish)API更新
                  $result = ndcop_dataset_api_harvest_update_dataset( $identifier, $ndcopdata_api_format_json_data_string, $publisher_apikey );

                  timer_stop($identifier);

                  ndcop_api_log_harvest_result('Harvest_Update', $harvest_api_sru, $harvest_api_sru.'/rest/dataset/'. $identifier , $result['id'], $result['ndc_harvest_result'] , $ndcopdata_api_format_json_data_string );

                  watchdog('ndcop_dataset_api','Harvest更新詮釋資料結束, id=%id, time=%time, result=%result', array('%id' => $identifier,'%time' => (timer_read($identifier)/1000).'s','%result'=>$result, WATCHDOG_DEBUG));
              }
          }
      }
      /*----------------------------------------------------------------*/


        // 比對其他開放平臺所有清單(不帶參數)和NDC政府資料開放平臺清單
        // 將NDC政府資料開放平臺清單和其他開放平臺所有清單同步成一致
      /*----------------------------------------------------------------*/
        // 再次取得NDC政府資料平臺上dataset 的API IN identifier集合.
      module_load_include('inc', 'ndcop_api', 'ndcop_api.common');
      $apiin_id_set = ndcop_api_get_apiin_dataset_id( $param['oid'] );

      $harvest_api_accesspoint = $harvest_api_sru.'/rest/dataset';
        // 判斷是否為政府資料開放平臺的SRU，加上參數oid
      if( isset( $param['oid'] ) and ndcop_dataset_api_check_ndcsite_sru( $harvest_api_sru ) ){
        $harvest_api_accesspoint .= ('?oid=' . $param['oid'] );
    }

        // 取得擷取機關平台上所有詮釋資料清單
    $retrieve_org_datalist_all = ndcop_dataset_api_retrieve_datalist( $harvest_api_accesspoint );

    watchdog('ndcop_dataset_api','Harvest access point: %access ; retrieve_org_datalist_all: %data',
       array('%access'=> $harvest_api_accesspoint,'%data' => print_r($retrieve_org_datalist_all,true), WATCHDOG_DEBUG));

        // 有取得回傳值
    if( !empty($retrieve_org_datalist_all) and !empty($apiin_id_set) ){
            // 取得擷取機關平台上所有詮釋資料清單(根據不同平台可能會有不同的取法)
        if( ndcop_dataset_api_check_ndcsite_sru( $harvest_api_sru ) ){
            $retrive_all_id_list = $retrieve_org_datalist_all->result;
        }
        else{
            $retrive_all_id_list = $retrieve_org_datalist_all;
        }
            // NDC政府資料開放平臺清單集合和擷取機關平台上所有詮釋資料清單集合做差集
        $delete_ids = array_diff($apiin_id_set, $retrive_all_id_list);

            // 擷取機關平台上所有詮釋資料清單集合和NDC政府資料開放平臺清單集合做差集
        $create_ids = array_diff($retrive_all_id_list, $apiin_id_set);

        if ( !empty($delete_ids) ) {
            foreach ($delete_ids as $key => $identifier) {

                    // 移除詮釋資料
                watchdog('ndcop_dataset_api','Harvest 移除詮釋資料開始, id=%id', array('%id' => $identifier, WATCHDOG_DEBUG));
                timer_start($identifier);

                    // $publisher_apikey = '8fec9114-0e6b-4cbe-80dc-5964ef65d818';
                $publisher_apikey = $param['publish_api_key'];
                    // 呼叫資料集異動(Publish)API刪除
                $result = ndcop_dataset_api_harvest_delete_dataset( $identifier, $publisher_apikey );

                timer_stop($identifier);

                ndcop_api_log_harvest_result('Harvest_Delete', $harvest_api_sru, $harvest_api_sru.'/rest/dataset/'. $identifier , $result['id'], $result['ndc_harvest_result'] , $ndcopdata_api_format_json_data_string );

                watchdog('ndcop_dataset_api','Harvest 移除詮釋資料結束, id=%id, time=%time, result=%result', array('%id' => $identifier,'%time' => (timer_read($identifier)/1000).'s','%result'=>$result, WATCHDOG_DEBUG));
            }
        }

        if ( !empty($create_ids) ) {
            foreach ($create_ids as $key => $identifier) {
                    // 新增詮釋資料
                watchdog('ndcop_dataset_api','Harvest 新增詮釋資料開始, id=%id', array('%id' => $identifier, WATCHDOG_DEBUG));
                timer_start($identifier);

                    // 取得詮釋資料(必須為NDC規範格式的JSONDATA)
                    // 若Harvest進來的JSON不為NDC規範格式的JSONDATA，必須先轉換成NDC規範格式的JSONDATA
                $ndcopdata_api_format_json_data_string = ndcop_dataset_api_retrieve_data( $harvest_api_sru, $identifier );

                $publisher_apikey = $param['publish_api_key'];
                    // 呼叫資料集異動(Publish)API新增
                $result = ndcop_dataset_api_harvest_create_dataset( $ndcopdata_api_format_json_data_string, $publisher_apikey );

                timer_stop($identifier);

                ndcop_api_log_harvest_result('Harvest_Create', $harvest_api_sru, $harvest_api_sru.'/rest/dataset/'. $identifier , $result['id'], $result['ndc_harvest_result'] , $ndcopdata_api_format_json_data_string );

                watchdog('ndcop_dataset_api','Harvest 新增詮釋資料結束, id=%id, time=%time, result=%result', array('%id' => $identifier,'%time' => (timer_read($identifier)/1000).'s','%result'=>$result, WATCHDOG_DEBUG));
            }
        }
    }


    /*----------------------------------------------------------------*/
    watchdog('ndcop_dataset_api','資料集詮釋資料API定期擷取結束! Use Time: %time',
       array('%time' => timer_read($name).'ms', WATCHDOG_DEBUG));

    return TRUE;
    } // try
    catch(Exception $e){
        watchdog('ndcop_dataset_api','資料集詮釋資料API定期擷取發生Exception! %msg',
           array('%msg' => $e->getMessage(), WATCHDOG_DEBUG));
        return FALSE;
    }

}

/**
 *  利用其他機關提供的api url擷取資料集清單(不帶參數modified)
 *  @param $harvest_api_accesspoint: 擷取機關API
 */
function ndcop_dataset_api_retrieve_datalist( $harvest_api_accesspoint ){

	header("Content-type: application/json");
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 8);
    curl_setopt($ch, CURLOPT_TIMEOUT, 20);
    curl_setopt($ch, CURLOPT_URL, $harvest_api_accesspoint);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    $result = curl_exec($ch);
    curl_close($ch);
    $datalist_array = json_decode( ndcop_dataset_api_remove_bom( $result ) );
    return $datalist_array;
}

/**
 *  利用其他機關提供的api url擷取資料集詮釋資料
 *  @param $harvest_api_sru: 擷取機關API SRU
 *  @param $identifier: 擷取機關平台的identifier
 *  @return
 */
function ndcop_dataset_api_retrieve_data( $harvest_api_sru, $identifier ){

    header("Content-type: application/json");
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 2);
    curl_setopt($ch, CURLOPT_TIMEOUT, 20);
    curl_setopt($ch, CURLOPT_URL, $harvest_api_sru.'/rest/dataset/'. $identifier );
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    $result = curl_exec($ch);
    curl_close($ch);


    // 取得真正的資料集詮釋資料(根據不同平台可能會有不同的取法)
    $decode_result = json_decode( ndcop_dataset_api_remove_bom( $result ) );
    $ndcopdata_api_format_json_data_string = json_encode( $decode_result);
    $ndcopdata_api_format_json_data_string = ndcop_dataset_api_remove_bom( $ndcopdata_api_format_json_data_string ) ;
    return $ndcopdata_api_format_json_data_string;
}

/**
 *  若此程式碼在NDC政府資料開放平臺，呼叫NDC政府資料開放平臺本機的資料集異動(Publish) API
 *  若此程式碼在NDC多租戶平台，呼叫NDC多租戶平台本機的資料集異動(Publish) API
 *  @param $ndcopdata_api_format_json_data_string: 欲新增的詮釋資料JSON DATA
 *  @param $publisher_apikey: 機關API KEY
 *  @return (Array) id:
 *                  ndc_harvest_result: NDC平台新增的結果
 */
function ndcop_dataset_api_harvest_create_dataset( $ndcopdata_api_format_json_data_string, $publisher_apikey ){


    module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.dataset.modify');
    $ndc_opdata_format_obj = json_decode($ndcopdata_api_format_json_data_string);
    // print_r($ndc_opdata_format_obj);

    // LOG進來的JSON DATA
    watchdog('ndcop_dataset_api_harvest','SRU/rest/dataset(POST Method) Input json data: %json', array('%json' => $ndcopdata_api_format_json_data_string, WATCHDOG_DEBUG));

    // 判斷外部json data格式是否有誤
    $json_format_validate = ndcop_api_validate_json_format($ndc_opdata_format_obj);

    if( $json_format_validate['pass'] ){
        // 驗證API IN的JSON DATA
        $api_in_data_validate = ndcop_api_validate_api_in_data_process('create', $publisher_apikey, $ndcopdata_api_format_json_data_string);

        if( $api_in_data_validate['pass'] ){

            $authorization_api_key =  $publisher_apikey;
            $apikey_info = ndcop_api_check_apikey_info($authorization_api_key);
            // 新增詮釋資料
            $ndcop_api_id = ndcop_dataset_api_create_dataset($ndcopdata_api_format_json_data_string, $apikey_info);

            // ndcop_api_show_result_msg($ndcop_api_id, json_encode(''));
            return array('id'=>$ndcop_api_id, 'ndc_harvest_result'=>json_encode(''));

        }
        else{
            return array('id'=>$ndc_opdata_format_obj->identifier, 'ndc_harvest_result'=>json_encode($api_in_data_validate['error']));
        }

    }
    else{
        return array('id'=>$ndc_opdata_format_obj->identifier, 'ndc_harvest_result'=>json_encode($json_format_validate['error']) );
    }
}

/**
 *  若此程式碼在NDC政府資料開放平臺，呼叫NDC政府資料開放平臺本機的資料集異動(Publish) API
 *  若此程式碼在NDC多租戶平台，呼叫NDC多租戶平台本機的資料集異動(Publish) API
 *  @param $identifier: API IN的identifier
 *  @param $ndcopdata_api_format_json_data_string: 欲新增的詮釋資料JSON DATA
 *  @param $publisher_apikey: 機關API KEY
 *  @return (Array) id:
 *                  ndc_harvest_result: NDC平台更新的結果
 */
function ndcop_dataset_api_harvest_update_dataset( $identifier, $ndcopdata_api_format_json_data_string, $publisher_apikey ){


    module_load_include('inc', 'ndcop_dataset_api', 'ndcop_dataset_api.dataset.modify');
    $ndc_opdata_format_obj = json_decode($ndcopdata_api_format_json_data_string );
    // LOG進來的JSON DATA
    watchdog('ndcop_dataset_api','SRU/rest/dataset/{identifier}(PUT Method) Input json data: %json', array('%json' => $ndcopdata_api_format_json_data_string, WATCHDOG_DEBUG));

    // 判斷外部json data格式是否有誤
    $json_format_validate = ndcop_api_validate_json_format($ndc_opdata_format_obj);

    if( $json_format_validate['pass'] ){



        // 驗證API IN的JSON DATA
        $api_in_data_validate = ndcop_api_validate_api_in_data_process('update', $publisher_apikey, $ndcopdata_api_format_json_data_string, $identifier);

        if( $api_in_data_validate['pass'] ){
            $update_dataset_nid = $api_in_data_validate['dataset_nid'];

            $authorization_api_key =  $publisher_apikey;
            $apikey_info = ndcop_api_check_apikey_info($authorization_api_key);
            // 修改詮釋資料
            $ndcop_api_id = ndcop_dataset_api_update_dataset($ndcopdata_api_format_json_data_string, $apikey_info, $update_dataset_nid);

            return array('id'=>$ndcop_api_id, 'ndc_harvest_result'=>json_encode('') );
        }
        else{
            return array('id'=>$ndc_opdata_format_obj->identifier, 'ndc_harvest_result'=>json_encode($api_in_data_validate['error']) );
        }
    } // 判斷JSON Format End
    else{
        ndcop_api_show_result_msg($ndc_opdata_format_obj->identifier, json_encode($json_format_validate['error']), $ndcopdata_api_format_json_data_string );
        return array('id'=>$ndc_opdata_format_obj->identifier, 'ndc_harvest_result'=>json_encode($json_format_validate['error']) );
    }
}

/**
 *  若此程式碼在NDC政府資料開放平臺，呼叫NDC政府資料開放平臺本機的資料集異動(Publish) API
 *  若此程式碼在NDC多租戶平台，呼叫NDC多租戶平台本機的資料集異動(Publish) API
 *  @param $identifier: API IN的identifier
 *  @param $publisher_apikey: 機關API KEY
 *  @return (Array) id:
 *                  ndc_harvest_result: NDC平台刪除的結果
 */
function ndcop_dataset_api_harvest_delete_dataset( $identifier, $publisher_apikey ){


    $apikey_info = ndcop_api_check_apikey_info($publisher_apikey);
    $check_identifier = $identifier;
    $ndcop_id = ndcop_api_get_check_apiin_identifier($check_identifier, $apikey_info);
    $check_dataset_exist = ( !empty($ndcop_id) ) ? TRUE : FALSE ;

    if( $check_dataset_exist ){
        // 存在於政府資料開放平臺，可以刪除
        $dataset = node_load( $ndcop_id );
        $dataset->status = 0;
        // 永久下架(9)
        $dataset->field_dataset_status[LANGUAGE_NONE][0]['value'] = '9';
        $dataset->field_end_date[LANGUAGE_NONE][0]['value'] = time();
        // 透過API下架
        $dataset->field_dataset_modify_method[LANGUAGE_NONE][0]['value'] = '1';
        node_save($dataset);

        return array('id'=>$check_identifier, 'ndc_harvest_result'=>json_encode('') );
    }
    else{
        $error['error']['name'][] = '欲刪除的資料集不存在';
        $error['error']['__type'] = 'ER0060:欲刪除的資料集不存在';
        // ndcop_api_show_result_msg( $check_identifier, json_encode($error), $post_data_json_str );
        return array('id'=>$check_identifier, 'ndc_harvest_result'=>json_encode($error) );
    }
}

/**
 * 測試擷取
 * @see ndcop_dataset_api_exchang_org_harvest_setting_form_submit
 * @param $test_retrieve_api_url 資料集API網址
 * @return (boolean)測試成功 TRUE ; 測試失敗 FALSE
 */
function ndcop_dataset_api_test_harvest_api_sru( $test_retrieve_api_url ){

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);
    curl_setopt($ch, CURLOPT_TIMEOUT, 20);
    curl_setopt($ch, CURLOPT_URL, $test_retrieve_api_url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_ENCODING, 'UTF-8');
    $result = curl_exec($ch);
    curl_close($ch);


    // 若有BOM則移除
    if ( substr( $result, 0, 3 ) == "\xEF\xBB\xBF" ){
        $result=substr_replace( $result, '', 0, 3 ) ;
    }
    $json_decode_result = json_decode( trim($result), true);
    if(is_null($json_decode_result)){
      // if the json cannot be decoded
        return FALSE;
    }
    else{
        return TRUE;
    }
}

/**
 * Help Funciton 移除BOM
 * @param $result 移除BOM前的JSON
 * @return $result 移除BOM後的JSON
 */
function ndcop_dataset_api_remove_bom( $result ){

	// 若有BOM則移除
    if ( substr( $result, 0, 3 ) == "\xEF\xBB\xBF" ){
     $result = substr_replace( $result, '', 0, 3 ) ;
 }
 return $result;
}

/**
 * Help Funciton 檢查SRU是否為政府資料開放平臺的SRU
 * @param $harvest_api_sru 資料集API網址
 * @return (boolean)SRU為政府資料開放平臺的SRU TRUE ;
 *                  SRU非政府資料開放平臺的SRU FALSE
 */
function ndcop_dataset_api_check_ndcsite_sru( $harvest_api_sru ){

    return ( ( strpos($harvest_api_sru, 'data.gov.tw') !== false) or ( strpos($harvest_api_sru, 'data.nat.gov.tw') !== false) );
}
