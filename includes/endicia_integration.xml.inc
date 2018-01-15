<?php

/**
 * @file
 * Handles rate request/response related stuff for Endicia.
 */

/**
 * Builds an Endicia rate request.
 *
 * @param object $order
 *   The commerce order object.
 * @param object $shipping_address
 *   The commerce_customer_address array of the shipping profile.
 *
 * @return array
 *   An array of shipping rates.
 */
function endicia_integration_rate_request($order, $shipping_address) {
  $rates = array();

  $weight = endicia_integration_get_order_weight($order);
  $headers = array('Content-Type' => 'application/x-www-form-urlencoded');

  $xml = new SimpleXMLElement('<PostageRateRequest/>');
  $requester = $xml->addChild('RequesterID', variable_get('endicia_integration_account_id', ''));
  $certifiedIntermediary = $xml->addChild('CertifiedIntermediary');
  $accountID = $certifiedIntermediary->addChild('AccountID', variable_get('endicia_integration_account_id', ''));
  $passPhrase = $certifiedIntermediary->addChild('PassPhrase', variable_get('endicia_integration_passphrase', ''));
  $mailClass = $xml->addChild('MailClass', 'ParcelSelect');
  $weightOz = $xml->addChild('WeightOz', $weight);
  $mailpieceShape = $xml->addChild('MailpieceShape', 'Parcel');
  $fromPostalCode = $xml->addChild('FromPostalCode', substr(variable_get('endicia_integration_postal_code', ''), 0, 5));
  $toPostalCode = $xml->addChild('ToPostalCode', $shipping_address['postal_code']);
  $services = $xml->addChild('Services');
  $deliveryConfirmation = $services->addAttribute('DeliveryConfirmation', 'On');
  $responseOptions = $xml->addChild('ResponseOptions');
  $postagePrice = $responseOptions->addAttribute('PostagePrice', 'TRUE');

  drupal_alter('endicia_integration_rate_request', $request);

  // Submit the rate request to Endicia.
  $response = endicia_integration_api_request('postageRateRequestXML=' . $xml->asXML());

  if ($response->Status == 0) {
    $rates['endicia_rate_quote'] = array(
      'amount' => commerce_currency_decimal_to_amount((string) $response->PostagePrice['TotalAmount'], commerce_default_currency()),
      'currency_code' => commerce_default_currency(),
      'data' => array(),
    );
  }
  return $rates;
}

/**
 * Submits an API request to Endicia.
 *
 * @param string $request
 *   A request string.
 * @param string $message
 *   Optional log message.
 *
 * @return string
 *   XML string response from Endicia.
 */
function endicia_integration_api_request($request, $message = '') {

  endicia_integration_log('Submitting API request to Endicia. @message:<pre>@request</pre>', array('@message' => $message, '@request' => $request));

  $request_url = variable_get('endicia_integration_connection_address', 'https://labelserver.endicia.com/LabelService/EwsLabelService.asmx/CalculatePostageRateXML');

  // Send the request.
  $headers = array('Content-Type' => 'application/x-www-form-urlencoded');
  $response = drupal_http_request($request_url, array('method' => 'POST', 'data' => $request, 'headers' => $headers));

  // If we received a response.
  if (!empty($response->data)) {
    endicia_integration_log('Response code:@code<br />Response:<pre>@response</pre>', array('@code' => $response->code, '@response' => $response->data));
    return @simplexml_load_string($response->data);
  }
  else {
    endicia_integration_log('We did not receive a response from Endicia.', WATCHDOG_ERROR);
  }

  return FALSE;
}

/**
 * Get new passphrase from Endicia and save it in the variable table.
 *
 * @return string
 *   The new passphrase created by Endicia.
 */
function endicia_integration_get_new_passphrase() {
  $headers = array('Content-Type' => 'application/x-www-form-urlencoded');

  // Get old passphrase
  $oldPassphrase = variable_get('endicia_integration_passphrase', '');

  // Set new passphrase
  $chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
  $newPassphrase = substr(str_shuffle($chars),0,10);

  $xml = new SimpleXMLElement('<ChangePassPhraseRequest/>');
  $requester = $xml->addChild('RequesterID', variable_get('endicia_integration_account_id', ''));
  $requestID = $xml->addChild('RequestID', time());
  $certifiedIntermediary = $xml->addChild('CertifiedIntermediary');
  $accountID = $certifiedIntermediary->addChild('AccountID', variable_get('endicia_integration_account_id', ''));
  $passPhrase = $certifiedIntermediary->addChild('PassPhrase', $oldPassphrase);
  $newPassPhraseXML = $xml->addChild('NewPassPhrase', $newPassphrase);

  $xmlPost = 'changePassPhraseRequestXML=' . $xml->asXML();

  $result = drupal_http_request($connection_url, $headers, 'POST', $xmlPost);
  $response = simplexml_load_string($result->data);

  if ($response->Status == 0) {
    variable_set('endicia_integration_passphrase', $newPassphrase);
    drupal_set_message(t('New passphrase ' . $newPassphrase . ' has been generated and saved.'));
  }
  else {
    drupal_set_message(t('Error %errno: %errmsg', array('%errno' => $response->Status, '%errmsg' => $response->ErrorMessage)));
  }

  return $newPassphrase;
}
