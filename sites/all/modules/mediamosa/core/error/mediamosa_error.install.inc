<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Error installer.
 */

require_once 'mediamosa_error_db.class.inc';

// In case of update mode, we need to skip this file (else will give fatal
// error when upgrading from 2.x to 3.x.
if (defined('MAINTENANCE_MODE') && MAINTENANCE_MODE == 'update') {
  return;
}

// Needed for mediamosa_error.class.inc.
require_once DRUPAL_ROOT . '/sites/all/modules/mediamosa_sdk/mediamosa_sdk.class.inc';

require_once 'mediamosa_error.class.inc';


/**
 * Implements hook_install().
 */
function _mediamosa_error_install() {
  // Create our table(s).
  mediamosa::drupal_install_schema(mediamosa_error_db::TABLE_NAME);

  $messages = array(
    'HTTP_OK' => '',
    'HTTP_CREATED' => '',
    'HTTP_NO_CONTENT' => '',

    'HTTP_UNAUTHORIZED' => '',
    'HTTP_FORBIDDEN' => '',
    'HTTP_NOT_FOUND' => 'Page not found: @uri (method: @method, parameters: @params)',

    'ERRORCODE_OKAY' => '',
    'ERRORCODE_TIME_RESTRICTION_START' => 'There is a time_restriction_start error: @date (@timestamp)',
    'ERRORCODE_TIME_RESTRICTION_END' => 'There is a time_restriction_end error: @date (@timestamp)',
    'ERRORCODE_FILE_NOT_FOUND' => "The file '@filename' does not exist",
    'ERRORCODE_DIR_NOT_FOUND' => "Directory '@location'not found / doesn't exists",
    'ERRORCODE_DIR_NOT_WRITABLE' => "Directory '@location' is read-only, need write access",
    'ERRORCODE_STREAMING_PROFILE_NOT_FOUND' => "No streaming server profile found for the video container used in this mediafile. (video container: '@video_container')",
    'ERRORCODE_NO_MEDIAFILE_FOUND_FOR_PROFILE_ID' => 'No mediafile found for the given asset and/or profile_id',
    'ERRORCODE_QUERY_ERROR' => 'Incorrect database query',
    'ERRORCODE_STREAM_DOWNLOAD_NOT_ALLOWED' => 'Download not allowed for external streams',
    'ERRORCODE_NO_METAFILE_AVAILABLE' => "This container type doesn't support metafiles (@container_type)",
    'ERRORCODE_JOBS_COULD_NOT_BE_STOPPED' => 'Some jobs could not be stopped. Please try again later',
    'ERRORCODE_REST_CALL_IS_DISABLED' => "Rest call to '@uri' has been disabled, please check your MediaMosa documentation",
    'ERRORCODE_UNABLE_TO_CREATE_SYMLINK' => 'Unable to create symbolic link ticket to file',
    'ERRORCODE_REST_UNSPECIFIED_VARIABLE' => "The var '@var' is not allowed in this REST call, remove it from your request",
    'ERRORCODE_REST_DIFF_VALUE_GET_POST_VAR' => '@var is specified more than once (GET / POST / URI), but has different values (value 1; @value1 / value 2; @value2)',
    'ERRORCODE_REST_NOT_FOUND' => 'The REST call with uri_id @uri_id was not found',
    'ERRORCODE_DIR_UNABLE_TO_CREATE' => "Unable to create directory '@location', reason: @reason",
    'ERRORCODE_MISSING_EXTENSION' => "Unable to complete request, missing/requires '@extension'",
    'ERRORCODE_NO_MEDIAFILE_FOUND_FOR_TAG' => "No mediafile found for the given asset and tag",

    'ERRORCODE_ASSET_NOT_FOUND' => "The asset with ID '@asset_id' was not found in the database",
    'ERRORCODE_MEDIAFILE_NOT_FOUND' => "The mediafile with ID '@mediafile_id' was not found in the database",
    'ERRORCODE_INVALID_APP_ID' => 'Invalid application id (@app_id)',
    'ERRORCODE_INVALID_USER_ID' => 'The specified user_id (@user_id) is not the owner of this asset',
    'ERRORCODE_UNKNOWN_ERROR_SAVING_METADATA' => "Unknown error while saving metadata for asset with '@asset_id'",
    'ERRORCODE_EMPTY_RESULT' => 'The query returned an empty result set (i.e. zero rows)',
    'ERRORCODE_COLLECTION_NOT_FOUND' => "No collection with ID '@coll_id' was not found in the database",
    'ERRORCODE_COLLECTION_ASSET_RELATION_ALREADY_EXISTS' =>  'The asset is already present in the collection',
    'ERRORCODE_COLLECTION_ASSET_RELATION_NOT_FOUND' => "The asset with ID: '@asset_id' with collection ID '@coll_id' relation was not found in the database",
    'ERRORCODE_ASSET_NOT_EMPTY' => 'There are still @mediafile_count mediafile(s) present in this asset',
    'ERRORCODE_COLLECTION_NOT_EMPTY' => 'There are still @asset_count asset(s) present in this collection',
    'ERRORCODE_ASSET_SUPPLEMENT_NOT_FOUND' => "Supplement with ID '@supplement_id' was not found.",
    'ERRORCODE_WEBSERVICE_DISABLED' => 'The webservices has been disabled',
    'ERRORCODE_UNKNOWN_MEDIAMOSA_VERSION' => 'Unknown MediaMosa version requested: @version',
    'ERRORCODE_INVALID_ASSET_MEDIAFILE_COMBINATION' => 'Invalid asset and mediafile combination',
    'ERRORCODE_NO_CHANGES' => 'Insufficient paramaters given, changes are not saved',
    'ERRORCODE_INVALID_FAV_TYPE' => 'Invalid favorite type (@type) found, use one of these valid favorite types; (@valid_types)',
    'ERRORCODE_INVALID_TIMESTAMP' => 'The datetime @name is not a valid datetime or timestamp value, must be in format of YYYY-MM-DD HH:MM:DD / YYYY-MM-DD HH:MM / YYYY-MM-DD HH / YYYY-MM-DD / YYYY-MM / YYYY',
    'ERRORCODE_UNEXPECTED_ERROR' => 'A unexpected program error / problem happened',
    'ERRORCODE_MISSING_TIMESTAMP' => '@name must have 2 dates separated with a |',
    'ERRORCODE_CANT_PLAY_MEDIAFILE' => 'This mediafile (@mediafile_id) can not be played; @reason',
    'ERRORCODE_CANT_TRANSCODE_MEDIAFILE' => 'This mediafile (@mediafile_id) can not be transcoded; @reason',
    'ERRORCODE_APP_DISABLED' => 'This application has been disabled',
    'ERRORCODE_MEDIAFILE_IS_NOT_ORIGINAL' => 'The mediafile (@mediafile_id) is not original',
    'ERRORCODE_INVALID_MEDIAFILE_RELATION' => 'The mediafile (@mediafile_id) is not the parent of the new original mediafile (@new_original)',

    'ERRORCODE_UNKNOWN_JOB' => 'Unknown job specified',

    'ERRORCODE_TEST_RUNNING' => 'Timeout: test "@test" is currently running',

    'ERRORCODE_VALIDATE_INVALID_TYPE' => 'Invalid type specified: @type',
    'ERRORCODE_VALIDATE_INVALID_LENGTH_USE' => "'length' can not be used in combination with 'length_(min|max)'",
    'ERRORCODE_VALIDATE_REQUIRED_PARAMETER' => 'Required parameter @param (@type) is missing',
    'ERRORCODE_VALIDATE_FAILED' => 'Parameter validation failed for @param (@type)',
    'ERRORCODE_SORT_FIELD_ERROR' => "Sorting on '@field' is not allowed",
    'ERRORCODE_SORT_DIRECTION_ERROR' => "Invalid sort direction specified: '@sort_direction'. Use 'ASC' or 'DESC' instead",
    'ERRORCODE_PP_INVALID_TIME' => 'Parameter validation failed for @param (@type); must be less than 87 400 000 msec (=24h)',
    'ERRORCODE_VALIDATE_INVALID_LANGUAGE' => "Parameter validation for 'language' failed. Use the 2-letter codes as defined in ISO 639-1",
    'ERRORCODE_INVALID_SUPPLEMENT' => 'Invalid XML or maximum allowed size exceeded',
    'ERRORCODE_MIX_OF_URI_AND_FILE' => 'Mediafiles can not have a URI and a filename',
    'ERRORCODE_CHANGE_URI_AND_FILE' => 'A URI-based mediafile can be changed to a file-based mediafile and vice versa',
    'ERRORCODE_CHANGE_OWNERSHIP_MISSING_PARAMETERS' => "No 'old' or 'new' value present",
    'ERRORCODE_INSUFFICIENT_PARAMETERS' => 'Insufficient parameters given',
    'ERRORCODE_SUPPLEMENT_LIMIT_REACHED' => 'Maximum number of supplements for this assed reached (@max)',
    'ERRORCODE_MEDIAFILE_DOWNLOAD_DISABLED' =>  'Download for this mediafile is disabled',
    'ERRORCODE_UPLOAD_TARGET_IS_NOT_AN_ORIGINAL_FILE' => 'Uploading to a non-original mediafile is not allowed',
    'ERRORCODE_QUOTA_REACHED' => 'Quota limit reached. Allowed size: @quota MB. Used size: @used MB',
    'ERRORCODE_HOSTNAME_ALREADY_IN_GROUP' =>  'The hostname (@hostname) is already a member of the autorisation group (@group)',
    'ERRORCODE_HOSTNAME_NOT_FOUND' => 'The hostname (@hostname) was not found in the autorisation group (@group)',
    'ERRORCODE_GROUP_NOT_EMPTY' => 'There are still one or more hostnames present in this autorisation group (@group)',
    'ERRORCODE_VALIDATE_SEARCH_SYNTAX_FAILED' => 'Search parameters syntax contains a syntax error, please check your search_* parameters',
    'ERRORCODE_VALIDATE_SEARCH_SYNTAX_FAILED_UNEXPECTED_END' => 'Search parameters syntax ends abruptly',
    'ERRORCODE_VALIDATE_SEARCH_SYNTAX_FAILED_INVALID_FIELD' => "Search parameters: you can search on field '@field'",
    'ERRORCODE_METADATA_DEFINITION_NOT_EMPTY' => 'There are still one or more properties present in this property definition',
    'ERRORCODE_INVALID_METADATA_DEFINITION' => 'Invalid metadata definition',
    'ERRORCODE_HOSTNAME_TYPE_NO_MATCH_GROUP_TYPE' => 'The hostname (@hostname) with type (@hostname_type) must be the same type as the group (@group) with type (@group_type)',
    'ERRORCODE_ACL_GROUP_ALREADY_EXISTS' => 'The authorization group (@group) already exists',
    'ERRORCODE_ACL_GROUP_NOT_FOUND' => 'The authorization group (@group) was not found',
    'ERRORCODE_ACL_NAME_NOT_FOUND' => 'The authorization hostname (@hostname) was not found',
    'ERRORCODE_INVALID_DATA_PARAMETER' => "The parameter '@parameter' contains invalid data, expecting @data",
    'ERRORCODE_FTP_BATCH_NOT_FOUND' => 'The FTP batch id (@batch_id) was not found',
    'ERRORCODE_CANT_RETYPE_GROUP_MUST_BE_EMPTY' => "You can not change the type of group '@group' when it contains hostnames",
    'ERRORCODE_CQL_EXCLUSIVE' => 'When you define a CQL search string, you can not specify any other search or order by parameter',
    'ERRORCODE_CQL_ERROR' => 'CQL returned an error; @error',
    'ERRORCODE_METADATA_DEFINITION_ALREADY_EXISTS' => 'Metadata definition already exists',
    'ERRORCODE_ACTION_AND_REPLACE' => "The 'action' and 'replace' vars can not both be set",
    'ERRORCODE_VALUE_MUST_START_WITH_ALPHABETIC_CHAR' => 'The parameter @param (@type) first character must be an alphabetic character',
    'ERRORCODE_VALIDATE_INT_TO_SMALL' => 'The value of @param is to low, must be at least @min_value',
    'ERRORCODE_VALIDATE_INT_TO_BIG' => 'The value of @param is to high, can not be bigger than @max_value',
    'ERRORCODE_VALIDATE_STRING_TO_SHORT' => 'The length of @param is to short, must be at least @min_length characters long',
    'ERRORCODE_VALIDATE_STRING_TO_LONG' => 'The length of @param is to long, can not be longer than @max_length characters',
    'ERRORCODE_VALIDATE_VALUE_NOT_ALLOWED' => "The value of @param (@type) is not allowed, allowed values are '@allowed_values'",
    'ERRORCODE_INTERNAL_ONLY' => 'You can not set REST parameter @param (@type), these used by internal REST calls',

    'ERRORCODE_RETRIEVING_JOBLIST' => 'Error retrieving joblist from the database',
    'ERRORCODE_RETRIEVING_JOBSTATUS' => 'Error retrieving jobstatus from the database',
    'ERRORCODE_WRITING_JOBSTATUS' => 'Error writing jobstatus to the database',
    'ERRORCODE_WRITING_JOBPROGRESS' => 'Error writing jobprogress to the database',
    'ERRORCODE_WRITING_UNKNOWN_JOBSTATUS' => 'Error writing jobstatus to the database, an invalid status code was sent',
    'ERRORCODE_RETRIEVING_TRANSCODELIST' => 'Error retrieving transcodelist from the database',
    'ERRORCODE_CREATING_JOB' => 'Error creating a new job',
    'ERRORCODE_DELETING_JOB' => 'Error deleting a job',
    'ERRORCODE_UNKNOWN_JOB_TYPE' => 'Unknown job type',
    'ERRORCODE_UNKNOWN_JOB_STATUS' => 'Unknown job status',
    'ERRORCODE_UNKNOWN_JOB_PROGRESS' => 'Unknown job progress',
    'ERRORCODE_RETRIEVING_TRANSCODE_PROFILE' => 'Error retrieving transcode profile from the database',
    'ERRORCODE_RETRIEVING_DEFAULT_TRANSCODE_PROFILE' => 'Error retrieving default transcode profile',
    'ERRORCODE_NO_DEFAULT_TRANSCODE_PROFILE' => 'No default transcode profile specified',
    'ERRORCODE_UNKNOWN_TRANSCODE_PROFILE' => 'Unknown transcode profile',
    'ERRORCODE_NO_TRANSCODE_PARAMETERS' => 'No transcoding parameters provided',
    'ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FOUND' => 'Transcoding parameter @key is unknown',
    'ERRORCODE_JOB_TRANSCODE_PARAMETER_TOO_LOW' => 'Transcoding parameter @key has a value of @value, which is lower then the minimum @min_value',
    'ERRORCODE_JOB_TRANSCODE_PARAMETER_TOO_HIGH' => 'Transcoding parameter @key has a value of @value, which is higher then the maximum @max_value',
    'ERRORCODE_JOB_TRANSCODE_PARAMETER_WRONG_VALUE' => 'Transcoding parameter @key has an invalid value of @value',
    'ERRORCODE_JOB_TRANSCODE_PARAMETER_NOT_FLOAT' => 'Transcoding parameter @key has an invalid value of @value',
    'ERRORCODE_JOB_TRANSCODE_PARAMETER_COMBINATION' => 'Transcoding parameter combination of tool @tool and file extension @file_extension does not exist',
    'ERRORCODE_JOB_TRANSCODE_TIMEOUT' => 'Transcode job has exceeded the max transcode job runtime (@timeout s)',
    'ERRORCODE_CREATING_TRANSCODE_JOB' => 'Unable to create transcode job',
    'ERRORCODE_JOB_NOT_FOUND' => "The job with '@job_id' was not found in the database",
    'ERRORCODE_JOB_ASSET_NOT_FOUND' => "Asset with asset '@asset_id' was not found in the database",
    'ERRORCODE_CREATING_UPLOAD_JOB' => 'Unable to create upload job',
    'ERRORCODE_CREATING_ANALYSE_JOB' => 'Unable to create analyse job',
    'ERRORCODE_JOB_MEDIAFILE_NOT_FOUND' => "Mediafile with ID '@mediafile_id' was not found in the database",
    'ERRORCODE_JOB_USER_NOT_FOUND' => "User with ID '@user_id' was not found in the database",
    'ERRORCODE_JOB_COULD_NOT_BE_REMOVED' => "Job could not be removed with ID '@job_id'",
    'ERRORCODE_JOB_FRAMETIME_GREATER_THEN_DURATION' => 'The chosen frametime is higher than duration of the mediafile',
    'ERRORCODE_RETRIEVING_ASSET' => 'Error retrieving the asset from database',
    'ERRORCODE_UPLOAD_ALREADY_EXISTS' => 'A upload is already active for this asset_id',
    'ERRORCODE_STILL_NOT_FOUND' => "Could not find still for asset with ID '@asset_id'",
    'ERRORCODE_SERVER_STILL_NOT_FOUND' => 'No default still server found',
    'ERRORCODE_SERVER_UPLOAD_NOT_FOUND' => 'Unable to find active upload server',
    'ERRORCODE_STILL_NOT_IMAGE' => "Still is not an image for asset with ID '@asset_id'",
    'ERRORCODE_STILL_IS_NOT_CREATABLE' => "Still can not be created for mediafile with '@mediafile_id'",
    'ERRORCODE_CREATING_DERIVATIVE' => "Unable creating derivative from image with style: @style, path: @path",

    'ERRORCODE_STARTING_JOB_FAILED' => 'Jobserver is unable to start job',

    'ERRORCODE_MASTERSLAVE_DISALLOWED' => 'Master/slave relations for @acl_id disabled',

    'ERRORCODE_INVALID_UPLOAD_TICKET' => 'Upload failed, invalid upload ticket used',
    'ERRORCODE_CREATE_MEDIAFILE_DURING_UPLOAD' => 'Upload failed, unable to create mediafile during upload',
    'ERRORCODE_CANNOT_COPY_MEDIAFILE' => 'Upload failed, unable to copy mediafile to final destination',
    'ERRORCODE_NOT_ENOUGH_FREE_QUOTA' => 'Upload failed, not enough free quota',

    'ERRORCODE_DBUS_PROTOCOL_ERROR' => 'Authentication failed, DBUS protocol error',
    'ERRORCODE_ACCESS_DENIED' => 'Authentication failed, access denied',
    'ERRORCODE_ACCESS_DENIED_INVALID_APP_ID' => 'Authentication failed, access denied (invalid application ID)',

    'ERRORCODE_FTP_CREDENTIAL_LENGTH' => 'FTP user credentials are to short in length',
    'ERRORCODE_FTP_UNKNOWN_USER' => 'Unknown FTP user',
    'ERRORCODE_FTP_USER_EXISTS' => 'FTP user already exists',

    'ERRORCODE_NOT_AUTHORIZED' => 'Action is not allowed, not authorized (@reason)',

    'ERRORCODE_USERMAN_INVALID_GROUP' => 'Invalid group name',
    'ERRORCODE_USERMAN_GROUP_NOT_EMPTY' => 'The group is not empty',
    'ERRORCODE_USERMAN_INVALID_USER' => 'Invalid username',
    'ERRORCODE_USERMAN_GROUP_EXISTS' => 'Group already exists',
    'ERRORCODE_USERMAN_USER_EXISTS' => 'User already exists',
    'ERRORCODE_IS_UNAPPROPRIATE' => 'This video is flagged unappropriate',
    'ERRORCODE_MASTERSLAVE_OWN_APP' => 'You are not allowed to create a master-slave application record where the master and slave application are the same',
    'ERRORCODE_IMAGE_FILE_TOO_BIG' => 'The image file size is bigger, than the allowed maximum (@filesize byte)',
    'ERRORCODE_TRANSCODE_PROFILE_EXISTS' => "Transcode profile with name '@profile_name' already exists",
    'ERRORCODE_TRANSCODE_PROFILE_NOT_FOUND' => "Transcode profile with ID '@profile_id' was not found",
    'ERRORCODE_OPENAPI_MISSING_OPEN_APP_ID' => "Open client application is missing for OpenAPI",
    'ERRORCODE_INVALID_REST_CALL' => "Invalid REST call with ID '@rest_call'",
    'ERRORCODE_INVALID_TICKET' => "Invalid ticket with ID '@ticket_id'",
  );

  $query = array();
  $args = array();
  $x = 0;
  foreach ($messages as $name => $message) {
    $code = constant('mediamosa_error::' . $name);
    assert($code);

    $query[] = '(:name_' . $x . ', :code_' . $x . ', :message_' . $x . ')';
    $args[':name_' . $x] = $name;
    $args[':code_' . $x] = $code;
    $args[':message_' . $x] = $message;
    $x++;
  }

  // Now insert our errors.
  db_query('INSERT INTO {mediamosa_error} (name, code, message) VALUES ' . implode(",\n", $query), $args);
}

/**
 * Implements hook_install().
 */
function _mediamosa_error_uninstall() {
  // Drop our table(s).
  mediamosa::drupal_uninstall_schema(mediamosa_error_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_error_schema() {
  $schema[mediamosa_error_db::TABLE_NAME] = array(
  'description' => 'Stores the error code and messages of the error handeling.',
    'fields' => array(
      mediamosa_error_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => 'Primary Key: Unique ID.',
      ),
      mediamosa_error_db::NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_error_db::NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The ID in capitals of the error code.',
      ),
      mediamosa_error_db::CODE => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'The error code number.',
      ),
      mediamosa_error_db::MESSAGE => array(
        'type' => 'text',
        'not null' => TRUE,
        'description' => 'The message to display.',
      ),
    ),
    'primary key' => array(mediamosa_error_db::ID),
    'unique keys' => array(
      'uni_' . mediamosa_error_db::NAME => array(mediamosa_error_db::NAME),
      'uni_' . mediamosa_error_db::CODE => array(mediamosa_error_db::CODE),
    ),
  );

  return $schema;
}
