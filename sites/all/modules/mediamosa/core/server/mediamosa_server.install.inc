<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Installer for mediamosa_server_download module.
 */

require_once 'mediamosa_server_db.class.inc';

require_once 'job/mediamosa_server_job.install.inc';
require_once 'streaming/mediamosa_server_streaming.install.inc';
require_once 'tool/mediamosa_server_tool.install.inc';

/**
 * Implements hook_install().
 */
function _mediamosa_server_install() {

  // Server node extensions.
  mediamosa::drupal_install_schema(mediamosa_server_db::TABLE_NAME);

  // Children.
  _mediamosa_server_streaming_install();
  _mediamosa_server_tool_install();

  $default_servers = array(
    array(1, 0, 'ON', 'streaming', 'http://mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'ticket/{TICKET}',
      'asf',
      "{IF_EXTERNAL}
<asx version='3.0'>
<entry>
<ref href='{TICKET_URI}' />
{IF_START}
<starttime value='{START_TIME}' />
{/IF_START}
{IF_DURATION}
<duration value='{DURATION_TIME}' />
{/IF_DURATION}
</entry>
</asx>
{/IF_EXTERNAL}
<object id='{MEDIAFILE_ID}' classid='CLSID:22D6F312-B0F6-11D0-94AB-0080C74C7E95' standby='Loading Media Player components...' type='application/x-oleobject' width='{WIDTH}' height='{HEIGHT}'>
<param name='filename' value='{WMA_TICKET_URI}' />
<param name='autostart' value='{AUTOPLAY}' />
<embed type='application/x-mplayer2' src='{WMA_TICKET_URI}' autostart='{AUTOPLAY_NUM}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}'/>
</embed>
</object>"
      , 'ASF Streaming Server', '', '', '', 'APC', ''),
    array(2, 0, 'ON', 'streaming', 'http://mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'ticket/{TICKET}',
      'flv',
          "<script type='text/javascript' src='" . url('', array('absolute' => TRUE)) . "mmplayer/swfobject.js'></script>
<div id='mediaspace'>This div will be replaced</div>
<script type='text/javascript'>
var s1 = new SWFObject('" . url('', array('absolute' => TRUE)) . "mmplayer/player.swf','ply','{WIDTH}','{HEIGHT}','9');
s1.addParam('allowfullscreen','true');
s1.addParam('allowscriptaccess','always');
s1.addParam('wmode','opaque');
s1.addParam('flashvars','file={TICKET_URI}&autostart={AUTOPLAY}{IF_START}&start={START_TIME_SECONDS}{/IF_START}{IF_DURATION}&duration={START_PLUS_DURATION_TIME_SECONDS}{/IF_DURATION}');
s1.write('mediaspace');
</script>"
      , 'FLV Streaming Server', '', '', '', 'APC', ''),
    array(3, 0, 'ON', 'streaming', 'http://mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'ticket/{TICKET}',
      'mov;mp4;m4a;3gp;3g2;mj2:h264',
          "<script type='text/javascript' src='" . url('', array('absolute' => TRUE)) . "mmplayer/swfobject.js'></script>
<div id='mediaspace'>This div will be replaced</div>
<script type='text/javascript'>
var s1 = new SWFObject('" . url('', array('absolute' => TRUE)) . "mmplayer/player.swf','ply','{WIDTH}','{HEIGHT_PLUS_20}','9');
s1.addParam('allowfullscreen','true');
s1.addParam('allowscriptaccess','always');
s1.addParam('wmode','opaque');
s1.addParam('flashvars','file={TICKET_URI}&autostart={AUTOPLAY}{IF_START}&start={START_TIME_SECONDS}{/IF_START}{IF_DURATION}&duration={START_PLUS_DURATION_TIME_SECONDS}{/IF_DURATION}');
s1.write('mediaspace');
</script>"
      , 'MOV/MP4/M4A/3GP/3G2/MJ2I:H264 Streaming Server', '', '', '', 'APC', ''),
    array(4, 0, 'ON', 'streaming', 'http://mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'ticket/{TICKET}',
      'mpeg|mov;mp4;m4a;3gp;3g2;mj2',
      "<object id='{MEDIAFILE_ID}' classid='CLSID:02Bf25D5-8C17-4B23-BC80-D3488ABDDC6B' standby='Loading Quicktime Player components...' width='{WIDTH}' height='{HEIGHT}'>
{IF_START}
<param name='starttime' value='{START_TIME}' />
{/IF_START}
{IF_DURATION}
<param name='endtime' value='{END_TIME}' />
{/IF_DURATION}
<param name='src' value='{TICKET_URI}' />
<param name='autoplay' value='{AUTOPLAY}' />
<embed type='video/quicktime' src='{TICKET_URI}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}' autoplay='{AUTOPLAY}' {IF_START}starttime='{START_TIME}'{/IF_START} {IF_DURATION}endtime='{END_TIME}'{/IF_DURATION}/>
</object>"
      , 'MPEG|MOV;MP4;M4A;3GP;3G2;MJ2 Streaming Server', '', '', '', 'APC', ''),

    array(5, 0, 'ON', 'streaming', 'http://mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'ticket/{TICKET}',
      'ogg',
      '<video id="video" src="{TICKET_URI}" height="{HEIGHT}" width="{WIDTH}" {AUTOPLAY_TEXT} controls="true">Your browser does not support this video</video>',
      'Apache - OGG', 0, '', '', 'APC', ''),
    array(6, 0, 'ON', 'streaming', 'http://mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'ticket/{TICKET}',
      'matroska;webm',
      '<video id="video" src="{TICKET_URI}" height="{HEIGHT}" width="{WIDTH}" {AUTOPLAY_TEXT} controls="true">Your browser does not support this video</video>'
      , 'Apache - WebM', 0, '', '', 'APC', ''),

    array(11, 2, 'ON', 'download', 'http://download.mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'download/{TICKET}', '', '', 'Default download server', 0, '', '', 'APC', ''),
    array(12, 0, 'ON', 'still', 'http://download.mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'still/{TICKET}', '', '', 'Default still server', 0, '', '', 'APC', ''),
    array(13, 0, 'ON', 'job_processor', 'http://job1.mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI, '', '', 'Default 1st job processor', 2, 'ANALYSE|ffmpeg|STILL', '', 'APC', ''),
    array(14, 0, 'OFF', 'job_processor', 'http://job2.mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI, '', '', 'Default 2nd job processor', 2, 'ANALYSE|ffmpeg|STILL', '', 'APC', ''),
    array(15, 0, 'ON', 'upload', 'http://upload.mediamosa.local/', mediamosa_server_db::SERVER_PATH_BASE_URI . 'mediafile/upload?upload_ticket={TICKET}', '', '', 'Default upload server', 0, '', 'http://upload.mediamosa.local/mediafile/uploadprogress?id={PROGRESS_ID}', 'APC', 'http://upload.mediamosa.local/progress?X-Progress-ID={PROGRESS_ID}'),
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
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER, $default_server[8]);

    $node->{mediamosa_server_db::VERSION} = $default_server[1];
    $node->{mediamosa_server_db::SERVER_STATUS} = $default_server[2];
    $node->{mediamosa_server_db::SERVER_TYPE} = $default_server[3];
    $node->{mediamosa_server_db::SERVER_URI} = $default_server[4];
    $node->{mediamosa_server_db::SERVER_PATH} = $default_server[5];
    $node->{mediamosa_server_db::CONTAINERS} = $default_server[6];
    $node->{mediamosa_server_db::OBJECT_CODE} = $default_server[7];
    $node->{mediamosa_server_db::DESCRIPTION} = $node->{mediamosa_server_db::SERVER_NAME} = $default_server[8];
    $node->{mediamosa_server_db::SLOTS} = (int)$default_server[9];
    $node->{mediamosa_server_db::TOOLS} = $default_server[10];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS} = $default_server[11];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER_TYPE} = $default_server[12];
    $node->{mediamosa_server_db::URI_UPLOAD_PROGRESS_SERVER} = $default_server[13];

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
    )
  );

  return $schema;
}
