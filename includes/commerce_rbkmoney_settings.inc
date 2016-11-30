<?php

/**
 * Define payment statuses
 */
define('RBKMONEY_STATUS_FAILED', 'failed');
define('RBKMONEY_STATUS_SUCCESS', 'paid');

define('COMPANY_NAME', 'RBKmoney');

define('MODULE_NAME', 'commerce_rbkmoney');
define('BASE_PATH_MODULE', base_path() . drupal_get_path('module', MODULE_NAME));

define('MERCHANT_TOKEN', variable_get('commerce_rbkmoney_token', 'merchant_token'));
define('MERCHANT_CDS_TOKEN', variable_get('commerce_rbkmoney_cds_token', 'cds_token'));

define('BASE_URL', $GLOBALS['base_url']);

define('PAYMENT_FORM_URL', BASE_URL . '/commerce/form/payment');
define('PAYMENT_FORM_INIT_URL', BASE_URL . '/commerce/form/init');
define('PAYMENT_FORM_STATUS_URL', BASE_URL . '/commerce/status');

define('COMMON_API_URL', variable_get('commerce_rbkmoney_capi_url', 'http://capi.service.consul:8080/v1/'));
define('PATH_JS_PAYFRAME', variable_get('commerce_rbkmoney_payframe_url', 'http://payform.rbkmoney.com:8080/payframe/payframe.js'));

define('PATH_IMG_LOGO', BASE_PATH_MODULE . '/images/rbkmoney.png');
define('PATH_IMG_LOGO_PAYMENT_FORM', BASE_URL . BASE_PATH_MODULE . '/images/logo_360.png');

define('DEFAULT_CURRENCY', 'RUB');
