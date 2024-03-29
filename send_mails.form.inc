<?php

/**
 * @file
 * Mail compose forms.
 */

/**
 * Implements hook_form().
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
      '#collapsed' => FALSE,
    );
    $form['advanced_send_option']['from_name'] = array(
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 255,
      '#required' => TRUE,
      '#title' => t('Mail Sender name'),
      '#default_value' => variable_get('site_name', ''),
      '#description' => t("Enter the sender name for the mail"),
    );
    $form['advanced_send_option']['from_id'] = array(
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 255,
      '#required' => TRUE,
      '#title' => t('Mail Sender address'),
      '#default_value' => variable_get('site_mail', ''),
      '#description' => t("Enter the From for the mail"),
    );
    $form['advanced_send_option']['use_bcc'] = array(
      '#type' => 'checkbox',
      '#title' => t('Bcc recievers'),
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
  $form['bcc_ids'] = array(
    '#type' => 'textarea',
    '#title' => t('Bcc'),
    '#required' => TRUE,
    '#rows' => 8,
    '#cols' => 60,
    '#description' => t("Enter one address per line"),
    '#attributes' => array('class' => array('text-full')),
    '#states' => array(
      'visible' => array(
        ':input[name="use_bcc"]' => array('checked' => TRUE),
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
    '#value' => t('Send'),
  );
  return $form;
}

/**
 * Implements hook_form_validate().
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
  if (!$form_state['values']['use_bcc'] && count($email)>0) {
    foreach ($email as $value) {
      if (!filter_var(trim($value), FILTER_VALIDATE_EMAIL)) {
        form_set_error('to_id', t('Invalid "To Address" please enter valid e-mail ids'));
      }
     }
  }
  if ($form_state['values']['use_bcc']) {
    $bemail = explode("\n", trim($form_state['values']['bcc_ids']));
    foreach ($bemail as $value) {
      if (!filter_var(trim($value), FILTER_VALIDATE_EMAIL)) {
        form_set_error('bcc_ids', t('Invalid "Bcc Address" please enter valid e-mail ids'));
        drupal_set_message($value, 'error');
      }
    }
  }
  if (user_access('advanced_send_mails')) {
    $from_id = trim($form_state['values']['from_id']);
    if (!filter_var($from_id, FILTER_VALIDATE_EMAIL)) {
      form_set_error('from_id', t('Invalid "From Address" please enter valid e-mail'));
    }
  }
}

/**
 * Implements hook_form_submit().
 */
function send_mails_form_submit(&$form, &$form_state) {
  global $user;
  // From Address from user login.
  if (user_access('advanced_send_mails')) {
    $from_id = $form_state['values']['from_id'];
    $from_name = $form_state['values']['from_name'];
    $from = empty($from_name) ? $from_id : sprintf("%s <%s>", $from_name, $from_id);

    // Bcc
    $use_bcc = $form_state['values']['use_bcc'];
  } else {
    if ($user->uid) {
      $from = $user->mail;
    } else {
      $from = variable_get('site_mail', '');
    }
    $use_bcc = false;
  }
  $module = 'send_mails';
  $language = language_default();
  $params = array();
  $file = file_load($form_state['values']['files']);
  $report = $form_state['values']['body']['value'];
  $send = FALSE;
  $key = 'send_mails_send';
  $email = $form_state['values']['to_id'];

  $message = drupal_mail($module, $key, $email, $language, $params, $from, $send);
  $message['subject'] = $form_state['values']['subject'];
  $message['body'] = array($report);

  if ($use_bcc) {
    $bccs = array();
    $bemail = explode("\n", $form_state['values']['bcc_ids']);
    foreach ($bemail as $value) {
      $bcc=trim($value);
      if (!filter_var($bcc, FILTER_VALIDATE_EMAIL)) {
        drupal_set_message(t('An error occurred and processing did not complete.'), 'error');
        drupal_set_message(filter_xss($value), 'error');
        $form_state['rebuild'] = TRUE;
        $form_state['error'] = TRUE;
        return;
      }
      $bccs[]=$bcc;
    }
    $message['headers']['Bcc']=implode(",", $bccs);
  }

  if (isset($file) && is_object($file)) {
    $message['params']['attachments'][] = array(
      'filepath' => $file->uri,
      'filename' => $file->filename,
      'filemime' => $file->filemime,
      'list' => TRUE,
    );
  }

  $system = drupal_mail_system($module, $key);
  $message = $system->format($message);
  $message['result'] = $system->mail($message);
  if ($message['result'])
    drupal_set_message(t('Your mail was sent successfully!'));
  else
    drupal_set_message(t('Unable to send e-mail. Contact the site administrator if the problem persists.'), 'error');
}
