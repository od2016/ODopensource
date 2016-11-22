<?php

/**
 * @file
 * 負責處理API共同設定的部分.
 * TODO:
 */

// 定義API共用變數
define('SRU', 'api/v1/');
define('SRU_LENGTH', 2);

// 測試API所需資訊
define('NDCOP_API_TEST_API_KEY', '8fec9114-0e6b-4cbe-80dc-5964ef65d717');
define('NDCOP_API_TEST_IP', '127.0.0.1');
define('NDCOP_API_TEST_ORG', '內政部');
define('NDCOP_API_TEST_API_IN_ID', 'C394AEDD-8261-4F75-AD0C-02543DF24CD7');
define('NDCOP_API_TEST_API_NDCOP_ID', '379000000a-000001');

// 詮釋資料欄位
define('FIELD_KEYWORD', 'field_keyword_g');
define('FIELD_SERIALNO', 'field_serialno');

define('NDCOP_API_CATEGORYCODE', 'field_code_name'); // 分類編號
define('NDCOP_API_NDCOP_IDENTIFIER', 'field_serialno'); // NDCOP資料集編號
define('NDCOP_API_APIIN_IDENTIFIER', 'field_dataset_in_id_g'); // APIIN資料集編號
define('NDCOP_API_DESCRIPTION', 'field_body'); // 資料集描述
define('NDCOP_API_FIELD_DESCRIPTION', 'field_field_fielddesc'); // 主要欄位說明
define('NDCOP_API_TYPE', 'field_datatype_name'); // 資料集類型
define('NDCOP_API_LICENSE', 'field_datalicense'); // 授權方式
define('NDCOP_API_LICENSE_URL', 'field_auth_link'); // 授權說明網址
define('NDCOP_API_COST', 'field_charge'); // 計費方式
define('NDCOP_API_COST_URL', 'field_paylink'); // 計費說明網址
define('NDCOP_API_COST_LAW', 'field_paylawsource'); // 計費法令依據
define('NDCOP_API_ORGANIZATION', 'field_second_org_fullname'); // 資料集所屬管理機關名稱
define('NDCOP_API_ORGANIZATION_OID', 'field_secord_org_oid'); // 資料集所屬管理機關OID

define('NDCOP_API_ORGANIZATION_CONTACTNAME', 'field_secord_org_contact_name'); // 資料集所屬管理機關聯絡人姓名
define('NDCOP_API_ORGANIZATION_CONTACTPHONE', 'field_secord_org_contact_phone'); // 資料集所屬管理機關聯絡人電話
define('NDCOP_API_ORGANIZATION_CONTACTEMAIL', 'field_secord_org_contact_email'); // 資料集所屬管理機關聯絡人電子郵件
define('NDCOP_API_PUBLISHER', 'field_orgfullname'); // 資料集提供機關名稱
define('NDCOP_API_PUBLISHER_OID', 'field_oid'); // 資料集提供機關OID
define('NDCOP_API_PUBLISHER_CONTACTNAME', 'field_contactname'); // 資料集提供機關聯絡人
define('NDCOP_API_PUBLISHER_CONTACTNAME_GENDER', 'field_sex'); // 資料集提供機關聯絡人稱謂
define('NDCOP_API_PUBLISHER_CONTACTPHONE', 'field_contactphone'); // 資料集提供機關聯絡人電話
define('NDCOP_API_PUBLISHER_CONTACTEMAIL', 'field_contactemail'); // 資料集提供機關聯絡人電子郵件
define('NDCOP_API_ACCRUALPERIODICITY', 'field_updatefreq'); //更新頻率
define('NDCOP_API_ISSUED', 'field_start_date'); //發布時間 (上架日期 in NDCOPDATA)
define('NDCOP_API_MODIFIED', 'field_version_update_time'); // 最後更新時間 (資料集修訂時間 in NDCOPDATA)
define('NDCOP_API_NUMBER_OF_DATA', 'field_amount'); //資料量
define('NDCOP_API_NOTES', 'field_note'); // 備註
define('NDCOP_API_RESOURCE', 'field_gov_resource'); // 資料資源(resource)計數
define('NDCOP_API_RESOURCE_COUNT', 'field_resource_count_g'); // 資料資源(resource)計數


define('NDCOP_API_TEMPORAL_COVERAGE_FROM', 'field_temporal_coverage_from_g'); // 開始收錄日期
define('NDCOP_API_TEMPORAL_COVERAGE_TO', 'field_temporal_coverage_to_g'); // 結束收錄日期
define('NDCOP_API_SPATIAL', 'field_spatial_g'); // 資料集空間範圍
define('NDCOP_API_LANGUAGE', 'field_language_g'); // 資料集語系
define('NDCOP_API_LANDING_PAGE', 'field_landing_page_g'); // 資料集相關網址
define('NDCOP_API_VIEW_COUNT','field_view_count'); //瀏覽次數
define('NDCOP_API_DOWNLOAD_COUNT','field_download_count_api'); //下載次數

// IN Field collections
define('NDCOP_API_RESOURCE_ID', 'field_resource_id_g'); // 資料項目編號
define('NDCOP_API_RESOURCE_APIIN_ID', 'field_resource_in_id_g'); // APIIN資料項目編號
define('NDCOP_API_RESOURCE_DESCRIPTION', 'field_resource_description_g'); // 資料項目描述
define('NDCOP_API_RESOURCE_FORMAT', 'field_format_g'); // 檔案格式
define('NDCOP_API_RESOURCE_CHARACTER_SET_CODE', 'field_character_set_code_g'); // 編碼格式
define('NDCOP_API_RESOURCE_MODIFIED', 'field_resource_modified_g'); // 資料項目最後更新時間
define('NDCOP_API_RESOURCE_URLTYPE', 'field_resource_url_type_g'); // 資料項目網址型態
define('NDCOP_API_RESOURCE_URL', 'field_resource_url_g'); // 資料項目網址
define('NDCOP_API_RESOURCE_METADATA_SOURCE', 'field_metadata_source_of_data_g'); // 領域別詮釋資料參考網址
define('NDCOP_API_RESOURCE_NID_REF', 'field_metadataset_nid_ref_g'); // 資料項目對應政府詮釋資料集NID

//Comments
define('NDCOP_API_COMMENT_AUTHOR','name');//意見回覆作者
define('NDCOP_API_COMMENT_TYPE','field_comment_type');//意見回覆類型
define('NDCOP_API_COMMENT_SUBJECT','subject');//意見回覆標題
define('NDCOP_API_COMMENT_BODY','comment_body');//意見回覆內容
define('NDCOP_API_COMMENT_TIME','field_comment_time');//意見回覆建立時間
define('NDCOP_API_COMMENT_CID','field_cid_api');//意見回覆編號
define('NDCOP_API_COMMENT_PID','field_pid_api');//意見回覆回應編號
define('NDCOP_API_COMMENT_HOST','field_comment_host');//意見回覆IP
