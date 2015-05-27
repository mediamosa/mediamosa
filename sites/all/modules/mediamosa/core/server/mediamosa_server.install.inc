<?php
/**
 * @file
 * Install, update, and uninstall functions for the mediamosa server module.
 */

require_once 'mediamosa_server_db.class.inc';

require_once 'job/mediamosa_server_job.install.inc';
require_once 'streaming/mediamosa_server_streaming.install.inc';
require_once 'tool/mediamosa_server_tool.install.inc';
require_once 'viewer/mediamosa_server_viewer.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_install() {

  // Check for https.
  $is_https = isset($_SERVER['HTTPS']) && strtolower($_SERVER['HTTPS']) == 'on';
  $scheme = $is_https ? 'https://' : 'http://';

  // Server node extensions.
  mediamosa::drupal_install_schema(mediamosa_server_db::TABLE_NAME);

  // Install the viewer table.
  _mediamosa_server_viewer_install();

  // Children.
  _mediamosa_server_streaming_install();
  _mediamosa_server_tool_install();

  // Get URL of the current server, stripped from scheme.
  $server_url = url('', array('absolute' => TRUE));
  $server_url = $scheme . rtrim(drupal_substr($server_url, strpos($server_url, '://') + 3), '/') . '/';

  $videojs_object = <<<VIDEOJS_OBJECT
<link href="@SERVER_URL/mmplayer/video-js/video-js.css" rel="stylesheet">
<script src="@SERVER_URL/mmplayer/video-js/video.js"></script>
<script>
  videojs.options.flash.swf = "@SERVER_URL/mmplayer/video-js/video-js.swf";
</script>

<video id="MediaMosa_video_1" class="video-js vjs-default-skin" width="{WIDTH}" height="{HEIGHT}" controls preload="auto" poster="{STILL_URI}" data-setup="{}">
  <source src="{TICKET_URI}" type='@VIDEO_TYPE' />
</video>
VIDEOJS_OBJECT;

  $startstop_code = <<<STARTSTOP_CODE

{IF_START}
<script>
videojs("MediaMosa_video_1").ready(function() {
  var gotoStart = function() {
    var myPlayer = this;
    if (myPlayer.currentTime() < {START_TIME_SECONDS}) {
      myPlayer.off("timeupdate", pauseAtEnd);
      myPlayer.off("timeupdate", gotoStart);
      myPlayer.currentTime({START_TIME_SECONDS});
      myPlayer.pause();
      myPlayer.bigPlayButton.show();
      myPlayer.on("timeupdate", pauseAtEnd);
    }
  };
  var pauseAtEnd = function() {
    var myPlayer = this;
{IF_DURATION}
    if (myPlayer.currentTime() > {START_PLUS_DURATION_TIME_SECONDS}) {
      myPlayer.pause();
      myPlayer.currentTime({START_TIME_SECONDS});
      myPlayer.bigPlayButton.show();
    }
{/IF_DURATION}
  };
  var myPlayer = this;
  myPlayer.on("timeupdate", gotoStart);
  myPlayer.play();
});
</script>
{/IF_START}
STARTSTOP_CODE;

  $pdf_object = '<iframe width="640" height="480" src="@SERVER_URL/pdf.js/web/viewer.html?file={TICKET_URI}" frameborder="0" allowfullscreen></iframe><a href=@SERVER_URL/pdf.js/web/viewer.html?file={TICKET_URI}>Open PDF in full screen</a>';

  $default_servers = array(

    // OGG.
    array(0, 0, 'ON', 'STREAMING', $server_url, mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}',
          'ogg', '',
          strtr($videojs_object . $startstop_code, array('@SERVER_URL' => $server_url, '@VIDEO_TYPE' => 'video/ogg')),
          'VideoJS player; ogg videos', 0, '', '', 'APC', ''),

    // Webm.
    array(1, 0, 'ON', 'STREAMING', $server_url,
          mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}',
          'matroska;webm', '',
          strtr($videojs_object . $startstop_code, array('@SERVER_URL' => $server_url, '@VIDEO_TYPE' => 'video/webm')),
          'VideoJS player; WebM Videos', 0, '', '', 'APC', ''),

    // Mp4.
    array(2, 0, 'ON', 'STREAMING', $server_url,
          mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}',
          'mov;mp4;m4a;3gp;3g2;mj2:h264', '',
          strtr($videojs_object . $startstop_code, array('@SERVER_URL' => $server_url, '@VIDEO_TYPE' => 'video/mp4')),
          'VideoJS player; mp4/h264 videos', 0, '', '', 'APC', ''),

    // Flv.
    array(3, 0, 'ON', 'STREAMING', $server_url,
          mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}',
          'flv', '',
          strtr($videojs_object, array('@SERVER_URL' => $server_url, '@VIDEO_TYPE' => 'video/flv')),
          'VideoJS player; flv videos', 0, '', '', 'APC', ''),

    // Most simple image viewer.
    array(4, 0, 'ON', 'STREAMING', $server_url,
          mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}',
          '', 'image/*',
          '<img id="mm_image_view_01" style="max-width:640px" src="{TICKET_URI}" alt="" />',
          'Image viewer', 0, '', '', 'APC', ''),

    // PDF viewer.
    array(5, 0, 'ON', 'STREAMING', $server_url,
          mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}',
          '', 'application/pdf',
          strtr($pdf_object, array('@SERVER_URL' => $server_url)),
          'PDF viewer with pdf.js', 0, '', '', 'APC', ''),

    array(6, 2, 'ON', 'DOWNLOAD', $scheme . 'download.' . $server_url, mediamosa_server_db::SERVER_PATH_BASE_URI . 'download/{TICKET}', '', '', '', 'Default download server', 0, '', '', 'APC', ''),
    array(7, 0, 'ON', 'STILL', $scheme . 'download.' . $server_url, mediamosa_server_db::SERVER_PATH_BASE_URI . 'media/{TICKET}', '', '', '', 'Default server for stills', 0, '', '', 'APC', ''),
    array(8, 0, 'ON', 'JOB_PROCESSOR', $scheme . 'job1.' . $server_url, mediamosa_server_db::SERVER_PATH_BASE_URI, '', '', '', 'Default 1st job processor', 2, 'TRANSFER|ANALYSE|ffmpeg|STILL', '', 'APC', ''),
    array(9, 0, 'OFF', 'JOB_PROCESSOR', $scheme . 'job2.' . $server_url, mediamosa_server_db::SERVER_PATH_BASE_URI, '', '', '', 'Default 2nd job processor', 2, 'TRANSFER|ANALYSE|ffmpeg|STILL', '', 'APC', ''),
    array(10, 0, 'ON', 'UPLOAD', $scheme . 'upload.' . $server_url, mediamosa_server_db::SERVER_PATH_BASE_URI . 'mediafile/upload?upload_ticket={TICKET}', '', '', '', 'Default upload server', 0, '', mediamosa_server_db::SERVER_PATH_BASE_URI . 'uploadprogress?id={PROGRESS_ID}', 'APC', mediamosa_server_db::SERVER_PATH_BASE_URI . 'progress?X-Progress-ID={PROGRESS_ID}'),
  );

  // Create our node type.
  $server_node_type = array(
    'type' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER,
    'name' => st('MediaMosa server'),
    'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER,
    'description' => st('MediaMosa servers execute jobs for transcoding, uploading and other related work.'),
    'custom' => 1,
    'modified' => 1,
    'locked' => 0,
    'has_body' => 0,
  );

  $server_node_type = node_type_set_defaults($server_node_type);
  node_type_save($server_node_type);

  foreach ($default_servers as $default_server) {
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER, $default_server[9]);

    $machine_name = empty($default_server[9]) ? 'mediamosa_server_' . $node->{mediamosa_server_db::SERVER_TYPE} : $default_server[9];
    $node->{mediamosa_server_db::MACHINE_NAME} = mediamosa_db::generate_machine_name($machine_name, mediamosa_server_db::TABLE_NAME);
    $node->{mediamosa_server_db::VERSION} = $default_server[1];
    $node->{mediamosa_server_db::SERVER_STATUS} = $default_server[2];
    $node->{mediamosa_server_db::SERVER_TYPE} = $default_server[3];
    $node->{mediamosa_server_db::SERVER_URI} = $default_server[4];
    $node->{mediamosa_server_db::SERVER_PATH} = $default_server[5];
    $node->{mediamosa_server_db::CONTAINERS} = $default_server[6];
    $node->{mediamosa_server_db::MIME_TYPE} = $default_server[7];
    $node->{mediamosa_server_db::OBJECT_CODE} = $default_server[8];
    $node->{mediamosa_server_db::DESCRIPTION} = $node->{mediamosa_server_db::SERVER_NAME} = $default_server[9];
    $node->{mediamosa_server_db::SLOTS} = (int)$default_server[10];
    $node->{mediamosa_server_db::TOOLS} = $default_server[11];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS} = $default_server[12];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE} = $default_server[13];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER} = $default_server[14];

    $node = node_save($node);
  }

  // Server job table is a link table between mediamosa_job and mediamosa_jobserver_job and our server.
  _mediamosa_server_job_install();
}

/**
 * Implements hook_uninstall().
 */
function _mediamosa_server_uninstall() {
  // Server job uninstall.
  _mediamosa_server_job_uninstall();

  // Uninstall the viewer table.
  _mediamosa_server_viewer_uninstall();

    // Unstall our children..
  _mediamosa_server_streaming_uninstall();
  _mediamosa_server_tool_uninstall();

  // Server node extensions.
  mediamosa::drupal_uninstall_schema(mediamosa_server_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_schema() {
  $types = array(
    mediamosa_server_db::SERVER_TYPE_DOWNLOAD,
    mediamosa_server_db::SERVER_TYPE_STILL,
    mediamosa_server_db::SERVER_TYPE_STREAMING,
    mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR,
    mediamosa_server_db::SERVER_TYPE_UPLOAD,
  );

  $server_status = array(
    mediamosa_server_db::SERVER_STATUS_OFF,
    mediamosa_server_db::SERVER_STATUS_ON,
    mediamosa_server_db::SERVER_STATUS_CLOSE,
  );

  $server_upload_progress_type = array(
    mediamosa_server_db::UPLOAD_PROGRESS_SERVER_TYPE_APC,
    mediamosa_server_db::UPLOAD_PROGRESS_SERVER_TYPE_PROXY,
  );

  $schema = array();
  $schema[mediamosa_server_db::TABLE_NAME] = array(
    'description' => 'The download server table contains settings for the download servers.',
    'fields' => array(
      mediamosa_server_db::NID => array(
        'description' => 'The Drupal node id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_db::VID => array(
        'description' => 'The Drupal node revision id.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_db::MACHINE_NAME => array(
        'type' => 'varchar',
        'length' => mediamosa_server_db::MACHINE_NAME_LENGTH,
        'not null' => TRUE,
        'default' => '',
        'description' => 'The MediaMosa server machine name.',
      ),
      mediamosa_server_db::VERSION => array(
        'description' => 'The version number.',
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => FALSE,
      ),
      mediamosa_server_db::SERVER_STATUS => array(
        'description' => 'The server status.',
        'type' => "enum('" . implode("','", $server_status) . "')",
        'mysql_type' => "enum('" . implode("','", $server_status) . "')",
        'not null' => TRUE,
      ),
      mediamosa_server_db::SERVER_TYPE => array(
        'description' => 'The server type.',
        'type' => "enum('" . implode("','", $types) . "')",
        'mysql_type' => "enum('" . implode("','", $types) . "')",
        'not null' => TRUE,
      ),
      mediamosa_server_db::SERVER_URI => array(
        'description' => 'The optional URI of the server.',
        'type' => 'varchar',
        'length' => mediamosa_server_db::SERVER_URI_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::SERVER_PATH => array(
        'description' => 'The path of the server.',
        'type' => 'varchar',
        'length' => mediamosa_server_db::SERVER_PATH_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::DESCRIPTION => array(
        'description' => 'The description of the server.',
        'type' => 'varchar',
        'length' => mediamosa_server_db::DESCRIPTION_LENGTH,
        'not null' => FALSE,
        'default' => '',
      ),
      mediamosa_server_db::CONTAINERS => array(
        'description' => 'The optional container names this server can serve.',
        'type' => 'text',
        'not null' => FALSE,
      ),
      mediamosa_server_db::MIME_TYPE => array(
        'description' => 'The optional mime type names this server can serve.',
        'type' => 'text',
        'not null' => FALSE,
      ),
      mediamosa_server_db::OBJECT_CODE => array(
        'description' => 'The optional container names the server can serve.',
        'type' => 'text',
        'not null' => FALSE,
      ),
      mediamosa_server_db::SERVER_NAME => array(
        'description' => 'The optional name of the server.',
        'type' => 'varchar',
        'length' => mediamosa_server_db::SERVER_NAME_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::SLOTS => array(
        'description' => 'The maximum number of concurrent transcodes.',
        'type' => 'int',
        'not null' => FALSE,
      ),
      mediamosa_server_db::TOOLS => array(
        'description' => 'The optional tools this server serves.',
        'type' => 'text',
        'not null' => FALSE,
      ),
      mediamosa_server_db::URI_UPLOAD_PROGRESS => array(
        'description' => 'The optional uri of the upload progress.',
        'type' => 'varchar',
        'length' => mediamosa_server_db::URI_UPLOAD_PROGRESS_LENGTH,
        'not null' => FALSE,
      ),
      mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER => array(
        'description' => 'The optional uri of the upload progress server side.',
        'type' => 'varchar',
        'length' => mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_LENGTH,
        'not null' => TRUE,
        'default' => '',
      ),
      mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE => array(
        'description' => 'The server type for upload progress server side.',
        'type' => "enum('" . implode("','", $server_upload_progress_type) . "')",
        'mysql_type' => "enum('" . implode("','", $server_upload_progress_type) . "')",
        'not null' => TRUE,
        'default' => mediamosa_server_db::UPLOAD_PROGRESS_SERVER_TYPE_APC,
      ),
    ),
    'primary key' => array(mediamosa_server_db::NID),
    'indexes' => array(
      'idx_nid_vid' => array(mediamosa_server_db::NID, mediamosa_server_db::VID),
      'idx_servertype_serverstatus' => array(mediamosa_server_db::NID, mediamosa_server_db::SERVER_TYPE, mediamosa_server_db::SERVER_STATUS),
    ),
    'unique keys' => array(
      'uni_machine_name' => array(mediamosa_server_db::MACHINE_NAME),
    ),
  );

  return $schema;
}
