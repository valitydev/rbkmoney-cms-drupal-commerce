<?php

/**
 * Init payments and redirect
 */
function commerce_rbkmoney_init_form_info()
{
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: *');
  header('Access-Control-Allow-Headers: Content-Type, Accept, Origin');

  if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
    http_response_code(200);
    exit();
  }

  $body = file_get_contents('php://input');

  logger(MODULE_NAME . '_init_payment_req', "<pre>%logs</pre>", ['%logs' => var_export($body, TRUE)]);

  $request = json_decode($body, true);

  if (!isset($request['invoiceId'])) {
    throw new Exception("Not found required parameter invoiceId");
  }

  $url = COMMON_API_URL . "processing/invoices/" . $request['invoiceId'] . '/payments';

  $data = [
    'paymentToolToken' => $request['token'],
    'paymentSession' => $request['session'],
    'contactInfo' => [
      'email' => !empty($request['contractInfo']['email']) ? $request['contractInfo']['email'] : '',
    ]
  ];

  $json = json_encode($data);

  $headers = [];
  $headers[] = 'X-Request-ID: ' . uniqid();
  $headers[] = 'Authorization: Bearer ' . MERCHANT_TOKEN;
  $headers[] = 'Content-type: application/json; charset=utf-8';
  $headers[] = 'Accept: application/json';

  // Init payment
  $response = send($url, 'POST', $headers, $json, 'init_payment');

  http_response_code($response['http_code']);
  print $response['body'];
  exit();
}
