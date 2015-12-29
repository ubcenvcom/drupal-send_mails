<?php

/**
 * @file
 * Mail compose forms.
 */

/**
 * Implements hook_form for Mail sender form.
 */
function send_mails_form() {
  
  $form = array('#attributes' => array('enctype' => 'multipart/form-data'));
  
  // Exclude anonymous user for rolebased.
  $user_roles = array_diff(user_roles(), array('anonymous user'));
  
  // Advanced send option fields.
  if (user_access('advanced_send_mails')) {
    $form['advanced_send_option'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced Send option'),
      '#collapsible' => TRUE, 
      '#collapsed' => TRUE,  
    );
    
    $form['advanced_send_option']['role_based'] = array(
      '#type' => 'checkbox',
      '#title' => t('Role based recievers'),
    );
    
    $form['advanced_send_option']['user_roles'] = array(
      '#type' => 'radios',
      '#options' => $user_roles,
      '#title' => t('Select required role'),
      '#states' => array(
        'visible' => array(
          ':input[name="role_based"]' => array('checked' => TRUE),
        ),
      ),
    );
    
  }
  
  $form['to_id'] = array(
    '#type' => 'textfield',
    '#autocomplete_path' => 'send-mails/mailids',
    '#size' => 60,
    '#maxlength' => 1000,
    '#title' => t('To Address'),
    '#description' => t("Enter the senders mail id separated by comma(,). Example: example@example.com,test@test.com,.."),
    '#states' => array(
      'invisible' => array(
        ':input[name="role_based"]' => array('checked' => TRUE),
      ),
      'required' => array(
        ':input[name="role_based"]' => array('checked' => FALSE),
      ),
    ),
  );
  
  $form['subject'] = array(
    '#type' => 'textfield',
    '#size' => 60,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#title' => t('Mail Subject'),
    '#description' => t("Enter the Subject for the mail"),
  );
  
  $form['body'] = array(
    '#type' => 'text_format',
    '#title' => t('Mail Body'),
    '#required' => TRUE,
    '#rows' => 20,
    '#cols' => 60,
    '#description' => t("Enter the Body content for mail"),
    '#attributes' => array('class' => array('text-full')),
  );
  
  $form['files'] = array(
    '#title' => t('Attachment'),
    '#type' => 'managed_file',
    '#name' => 'files[]',
    '#attributes' => array('multiple' => 'multiple'),
    '#upload_location' => 'public://sen_mails/',
    '#description' => t("Upload up to 5mb files."),
    '#upload_validators' => array(
        'file_validate_extensions' => array('zip tar tgz taz z gz rar gif png jpg jpeg doc xls ppt sxw sxc sxi sdw sdc sdd pdf'),
        'file_validate_size' => array(5000000),
    ),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send')
  );
  
  return $form;
}

/**
 * implements hook_form_validate().
 */

function send_mails_form_validate(&$form, &$form_state) {
   // Role based To address.
  if ($form_state['values']['role_based']) {
    $mailid_array = array();
    module_load_include('inc', 'send_mails', 'send_mails.query');
    $roles = $form_state['values']['user_roles'];
    $mailids = send_mails_query('mailid_from_role', $roles);
    foreach ($mailids as $value) {
      $mailid_array[] = $value->mail;
    }
    $form_state['values']['to_id'] = implode(",", $mailid_array);
  }
  $email = explode(',', $form_state['values']['to_id']);
  
  foreach ($email as $value) {
    if (!filter_var(trim($value), FILTER_VALIDATE_EMAIL)) {
      form_set_error('to_id', t('Invalid "To Address" please enter valid e-mail ids'));
    }
  }
  
  return;
}

/**
 *implements hook_form_submit()
 */
function send_mails_form_submit(&$form, &$form_state) {
  global $user;
  //From Address from user login
  if ($user->uid) {
    $from = $user->mail;  
  }
  else {
    $from = variable_get('site_mail', '');
  }
  $module = 'send_mails';
  $language = language_default();
  $params = array();
  $file = file_load($form_state['values']['files']);
  $report = $form_state['values']['body']['value'];
  $send = FALSE;
  $key = 'send_mails_send';
  $email = explode(',', $form_state['values']['to_id']);
  foreach ($email as $mails) {
    $message = drupal_mail($module, $key, trim($mails), $language, $params, $from, $send);
    $message['subject'] =  $form_state['values']['subject'];
    $message['body'] = array($report);
    $message['params']['attachments'][] = array(
      'filepath' => $file->uri,
      'filename' => $file->filename,
      'filemime' => $file->filemime,
      'list' => TRUE
    );
    $system = drupal_mail_system($module, $key);
    $message = $system->format($message);
    $message['result'] = $system->mail($message);
  }
  drupal_set_message(t('Your mail sent successfully!'));
}

