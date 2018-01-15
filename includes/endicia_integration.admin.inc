<?php

/**
 * @file
 * Admin functions for Endicia Integration.
 */

/**
 * Form builder function for module settings.
 *
 * @return string
 *   Drupal form for Endicia settings.
 */
function endicia_integration_settings_form() {
  $form['origin'] = array(
    '#title' => t('Ship from location'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
  );
  $form['origin']['endicia_integration_postal_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Postal Code'),
    '#default_value' => variable_get('endicia_integration_postal_code', ''),
  );
  $form['api'] = array(
    '#type' => 'fieldset',
    '#title' => t('Endicia Connection Settings'),
    '#collapsed' => variable_get('endicia_integration_user', FALSE),
    '#collapsible' => TRUE,
  );
  $form['api']['endicia_integration_account_id'] = array(
    '#type' => 'textfield',
    '#title' => t('AccountID'),
    '#default_value' => variable_get('endicia_integration_account_id', ''),
    '#description' => t('Fill in the specific AccountID provided by Endicia.'),
    '#required' => TRUE,
  );
  $form['api']['endicia_integration_passphrase'] = array(
    '#type' => 'textfield',
    '#title' => t('Passphrase'),
    '#default_value' => variable_get('endicia_integration_passphrase', ''),
    '#description' => t('Passphrase for your Endicia API.  This is not the same as your password to your Endicia web account.'),
    '#required' => TRUE,
  );
  $form['api']['endicia_integration_connection_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection Address'),
    '#default_value' => variable_get('endicia_integration_connection_address', 'https://labelserver.endicia.com/LabelService/EwsLabelService.asmx/CalculatePostageRateXML'),
    '#description' => t('Leave this set to https://labelserver.endicia.com/LabelService/EwsLabelService.asmx/CalculatePostageRateXML unless you have a reason to change it.'),
    '#required' => TRUE,
  );
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced options'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  $form['advanced']['endicia_integration_log'] = array(
    '#type' => 'radios',
    '#title' => t('Log messages from this module (useful for debugging)'),
    '#options' => array(
      0 => t('Do not log messages'),
      1 => t('Log messages'),
    ),
    '#default_value' => variable_get('endicia_integration_log', 0),
  );
  $form['advanced']['endicia_integration_shipping_string'] = array(
    '#type' => 'textfield',
    '#title' => t('Replace Shipping String Text'),
    '#default_value' => variable_get('endicia_integration_shipping_string', 'USPS Parcel Select'),
    '#description' => t('Replace the label text used to refer to the Endicia shipping option. This is useful if certain vendors require a specific string.'),
  );
  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 40,
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );
  return $form;
}

/**
 * Validate handler: valiates the settings form.
 */
function endicia_integration_settings_form_validate($form, $form_state) {
  // Check for a valid postal code.
  $postal_code = $form_state['values']['endicia_integration_postal_code'];
  if (!is_numeric($postal_code) || strlen($postal_code) != 5) {
    form_set_error('endicia_integration_postal_code', t('You must enter a 5 digit zip code'));
  }
}

/**
 * Submit handler: saves the settings.
 */
function endicia_integration_settings_form_submit($form, &$form_state) {
  // Exclude unnecessary elements.
  form_state_values_clean($form_state);

  foreach ($form_state['values'] as $key => $value) {
    if (is_array($value) && isset($form_state['values']['array_filter'])) {
      $value = array_keys(array_filter($value));
    }
    variable_set($key, $value);
  }

  drupal_set_message(t('The configuration options have been saved.'));
}
