<?php

/**
 * Prints the response invoice status.
 */
function uc_rbkmoney_status_info()
{
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: *');
  header('Access-Control-Allow-Headers: Content-Type, Accept, Origin');

  logger(MODULE_NAME . '_status_req', "<pre>%logs</pre>", ['%logs' => var_export($_GET, TRUE)]);

  $prepareQueryParams = [];
  $prepareQueryParams['limit'] = (!empty($_GET['limit'])) ? $_GET['limit'] : 100000;

  if (!empty($_GET['eventID'])) {
    $prepareQueryParams['eventID'] = $_GET['eventID'];
  }

  if (!isset($_GET['invoiceId'])) {
    throw new Exception("Not found required parameter invoiceId");
  }

  $capi_url = COMMON_API_URL . 'processing/invoices/' . $_GET['invoiceId'] . "/events?" . http_build_query($prepareQueryParams);

  $headers = [];
  $headers[] = 'X-Request-ID: ' . uniqid();
  $headers[] = 'Authorization: Bearer ' . MERCHANT_TOKEN;

  $response = send($capi_url, 'GET', $headers, '', 'polling_status');

  $events = drupal_json_decode($response['body']);

  $event_counting = count($events) - 1;
  $event = $events[$event_counting];
  $event_type = $event['eventType'];

  switch ($event_type) {
    case 'EventInvoiceStatusChanged':
    case 'EventPaymentStatusChanged':
      $status = $event['status'];
      break;

    default:
      $status = 'unknown';
  }

  if (in_array($status, ["paid", "failed", "cancelled"])) {
    if (isset($_GET['orderId'])) {
      $query = [
        'status' => $status,
        'order_id' => $_GET['orderId']
      ];
      $url = BASE_URL . '/commerce/rbkmoney/response?' . http_build_query($query);

      try {
        send($url, 'GET');
      } catch (Exception $ex) {
        logger(MODULE_NAME . '_exception', "<pre>%logs</pre>",
          ['%logs' => var_export(
            [
              'url' => $url,
              'query' => $query
            ],
            TRUE)
          ]);
      }
    }
  }

  http_response_code($response['http_code']);
  print $response['body'];
  exit();
}
