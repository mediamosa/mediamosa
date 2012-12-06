<?php
/**
 * @file
 * MediaMosa Rules with events, conditions and actions.
 */

/**
 * Implements hook_rules_event_info().
 *
 */
function mediamosa_rules_event_info() {
  $events = array();
  $defaults = array(
    'group' => t('MediaMosa'),
    'module' => 'mediamosa',
  );

  // Upload
  $events['mediamosa_event_upload_finished'] = $defaults + array(
    'label' => t('A upload was finished'),
    'variables' =>  mediamosa_rules_events_variable_mediafile_id(),
  );
  $events['mediamosa_event_upload_failed'] = $defaults + array(
    'label' => t('A upload has failed'),
    'variables' =>  mediamosa_rules_events_variable_mediafile_id(),
  );

  // Analyse.
  $events['mediamosa_event_analyse_finished'] = $defaults + array(
    'label' => t('A analyse was finished'),
    'variables' =>  mediamosa_rules_events_variable_mediafile_id(),
  );

  // Transcode.
  $events['mediamosa_event_transcode_started'] = $defaults + array(
    'label' => t('A transcode has started'),
    'variables' =>  mediamosa_rules_events_variable_mediafile_id(),
  );
  $events['mediamosa_event_transcode_finished'] = $defaults + array(
    'label' => t('A transcode finished'),
    'variables' =>  mediamosa_rules_events_variable_mediafile_id(),
  );
  $events['mediamosa_event_transcode_failed'] = $defaults + array(
    'label' => t('A transcode failed'),
    'variables' =>  mediamosa_rules_events_variable_mediafile_id(),
  );

  return $events;
}

/**
 * Implements hook_rules_condition_info().
 */
function mediamosa_rules_condition_info() {
  $conditions = array();
  /*
  $conditions['mediamosa_condition_mediafile_is_downloadable'] = array(
    'label' => t('Mediafile is downloadable'),
    'group' => t('MediaMosa'),
    'parameter' => array(
      'mediafile_id' => array(
        'type' => 'text',
        'label' => t('Mediafile ID'),
      ),
    ),
  );
  */
  return $conditions;
}

/**
 * Implements hook_rules_action_info().
 */
function mediamosa_rules_action_info() {
  $actions = array(
      'mediamosa_action_analyse' => array(
      'label' => t('Start an analyse job for a mediafile'),
      'group' => t('MediaMosa'),
      'parameter' => array(
        'mediafile_id' => array(
          'type' => 'text',
          'label' => t('Mediafile ID'),
        )
      ),
    ),
  );
  $actions['mediamosa_action_transcode'] = array(
    'label' => t('Start an transcode job for a mediafile'),
    'group' => t('MediaMosa'),
    'parameter' => array(
      'mediafile_id' => array(
        'type' => 'text',
        'label' => t('Mediafile ID'),
      ),
      'profile_id' => array(
        'type' => 'text',
        'label' => t('Profile ID'),
      ),
    ),
  );
  return $actions;
}

/**
 * Defines variable mediafile_id.
 */
function mediamosa_rules_events_variable_mediafile_id() {
  return array(
    'mediafile_id' => array(
      'type' => 'text',
      'label' => t('Mediafile'),
    ),
  );
}

/**
 * The action function for analyse.
 */
function mediamosa_action_analyse($mediafile_id) {
  // Start an analyse job.
}

/**
 * The action function for transcode.
 */
function mediamosa_action_transcode($mediafile_id, $profile_id) {
  // Start an transcode job.
}
