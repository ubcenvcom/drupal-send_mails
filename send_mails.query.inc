<?php

/**
 * @file
 * Database querys and Autocomplete json handlers.
 */

/**
 * Custom function to process database operations.
 */
function send_mails_query($type, $condition) {
  if ($type == 'mailid_from_role') {
    $query = db_select('users', 'u')
              ->fields('u', array('mail'));
    $query->condition('u.mail', '', '<>');
    // If Authenticated user.
    if ($condition != 2) {
      $query->join('users_roles', 'ur', 'ur.uid = u.uid');
      $query->condition('ur.rid', $condition);
    }
    $result = $query->execute()->fetchAll();
  }
  return $result;
}

/**
 * Custom Autocomplete field.
 */
function send_mails_mailids_autocomplete($string) {
  $matches = array();
  if ($string) {
    $array = drupal_explode_tags($string);
    // Fetch last string from user entered string.
    $last_string = trim(array_pop($array));
    $result = db_select('users')
      ->fields('users', array('mail'))
      ->condition('mail', '%' . db_like($last_string) . '%', 'LIKE')
      ->range(0, 10)
      ->execute();

    // Adding previous values along with last name.
    $prefix = count($array) ? implode(', ', $array) . ', ' : '';
    // Add matches to $matches.
    foreach ($result as $row) {
      if (!in_array($row->mail, $array)) {
        $matches[$prefix . $row->mail] = check_plain($row->mail);
      }
    }
  }
  // Return for JS.
  drupal_json_output($matches);
}
