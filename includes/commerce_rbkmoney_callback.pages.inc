<?php

/**
 * Callback URL for RBKmoney server response
 *
 * @see https://rbkmoney.github.io/webhooks-events-api/
 */
function commerce_rbkmoney_callback_result()
{
  $type = 'callback_result';
  $content = file_get_contents('php://input');
  _commerce_rbkmoney_logger($type, "<pre>%logs</pre>", ['%logs' => var_export(['content' => $content], TRUE)]);

  if (empty($_SERVER[SIGNATURE])) {
    http_response_code(HTTP_CODE_BAD_REQUEST);
    _commerce_rbkmoney_logger($type, "Webhook notification signature missing. <pre>%logs</pre>", ['%logs' => var_export(['content' => $content], TRUE)]);
    echo json_encode(array('message' => 'Webhook notification signature missing'));
    exit();
  }

  $params_signature = _commerce_rbkmoney_get_parameters_content_signature($_SERVER[SIGNATURE]);
  if (empty($params_signature[SIGNATURE_ALG])) {
    _commerce_rbkmoney_logger($type, "Missing required parameter " . SIGNATURE_ALG . " signature. <pre>%logs</pre>", ['%logs' => var_export(['content' => $content], TRUE)]);
    http_response_code(HTTP_CODE_BAD_REQUEST);
    echo json_encode(array('message' => 'Missing required parameter ' . SIGNATURE_ALG));
    exit();
  }

  if (empty($params_signature[SIGNATURE_DIGEST])) {
    _commerce_rbkmoney_logger($type, "Missing required parameter " . SIGNATURE_DIGEST . " signature. <pre>%logs</pre>", ['%logs' => var_export(['content' => $content], TRUE)]);
    http_response_code(HTTP_CODE_BAD_REQUEST);
    echo json_encode(array('message' => 'Missing required parameter ' . SIGNATURE_DIGEST));
    exit();
  }

  $signature = _commerce_rbkmoney_urlsafe_b64decode($params_signature[SIGNATURE_DIGEST]);
  if (!_commerce_rbkmoney_verification_signature($content, $signature, MERCHANT_CALLBACK_PUBLIC_KEY)) {
    _commerce_rbkmoney_logger($type, "Webhook notification signature mismatch. <pre>%logs</pre>",
      ['%logs' => var_export([
        $_SERVER[SIGNATURE],
        $content,
        $signature,
        MERCHANT_CALLBACK_PUBLIC_KEY
      ], TRUE)]);
    http_response_code(HTTP_CODE_BAD_REQUEST);
    echo json_encode(array('message' => 'Webhook notification signature mismatch'));
    exit();
  }

  $required_fields = [INVOICE, EVENT_TYPE];
  $data = json_decode($content, TRUE);

  foreach ($required_fields as $field) {
    if (empty($data[$field])) {
      _commerce_rbkmoney_logger($type, " One or more required fields are missing <pre>%logs</pre>",
        ['%logs' => var_export([
          'input_content' => $content,
          'required_fields' => $required_fields
        ], TRUE)]);
      http_response_code(HTTP_CODE_BAD_REQUEST);
      echo json_encode(array('message' => 'One or more required fields are missing'));
      exit();
    }
  }

  $current_shop_id = (int)variable_get('commerce_rbkmoney_shop_id');
  if ($data[INVOICE][INVOICE_SHOP_ID] != $current_shop_id) {
    _commerce_rbkmoney_logger($type, ' Webhook notification ' . INVOICE_SHOP_ID . ' mismatch<pre>%logs</pre>', ['%logs' => var_export($content, TRUE)]);
    http_response_code(HTTP_CODE_BAD_REQUEST);
    echo json_encode(array('message' => INVOICE_SHOP_ID . ' is missing'));
    exit();
  }

  if (empty($data[INVOICE][INVOICE_METADATA][ORDER_ID])) {
    _commerce_rbkmoney_logger($type, ORDER_ID . ' is missing <pre>%logs</pre>', ['%logs' => var_export($content, TRUE)]);
    http_response_code(HTTP_CODE_BAD_REQUEST);
    echo json_encode(array('message' => ORDER_ID . ' is missing'));
    exit();
  }

  $order_id = $data[INVOICE][INVOICE_METADATA][ORDER_ID];
  $order = commerce_order_load($order_id);
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  $order_amount = _commerce_rbkmoney_prepare_amount($order_wrapper);
  $invoice_amount = $data[INVOICE][INVOICE_AMOUNT];
  if ($order_amount != $invoice_amount) {
    _commerce_rbkmoney_logger($type, 'Received amount vs Order amount mismatch <pre>%logs</pre>', ['%logs' => var_export($content, TRUE)]);
    http_response_code(HTTP_CODE_BAD_REQUEST);
    echo json_encode(array('message' => 'Received amount vs Order amount mismatch'));
    exit();
  }

  $allowed_event_types = [EVENT_TYPE_INVOICE_PAID, EVENT_TYPE_INVOICE_CANCELLED];
  if (in_array($data[EVENT_TYPE], $allowed_event_types)) {
    $invoice_status = $data[INVOICE][INVOICE_STATUS];
    _commerce_rbkmoney_update_status_order($order_id, $invoice_status);
  }

  exit();
}

function _commerce_rbkmoney_urlsafe_b64decode($string)
{
  $data = str_replace(array('-', '_'), array('+', '/'), $string);
  $mod4 = strlen($data) % 4;
  if ($mod4) {
    $data .= substr('====', $mod4);
  }
  return base64_decode($data);
}

function _commerce_rbkmoney_urlsafe_b64encode($string)
{
  $data = base64_encode($string);
  return str_replace(array('+', '/'), array('-', '_'), $data);
}

function _commerce_rbkmoney_get_parameters_content_signature($content_signature)
{
  preg_match_all(SIGNATURE_PATTERN, $content_signature, $matches, PREG_PATTERN_ORDER);
  $params = array();
  $params[SIGNATURE_ALG] = !empty($matches[1][0]) ? $matches[1][0] : '';
  $params[SIGNATURE_DIGEST] = !empty($matches[2][0]) ? $matches[2][0] : '';
  return $params;
}

function _commerce_rbkmoney_verification_signature($data = '', $signature = '', $public_key = '')
{
  if (empty($data) || empty($signature) || empty($public_key)) {
    return FALSE;
  }

  $public_key_id = openssl_get_publickey($public_key);
  if (empty($public_key_id)) {
    return FALSE;
  }

  $verify = openssl_verify($data, $signature, $public_key_id, OPENSSL_SIGNATURE_ALG);
  return ($verify == OPENSSL_VERIFY_SIGNATURE_IS_CORRECT);
}
