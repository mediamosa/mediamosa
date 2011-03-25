TEST: Tags
FULL: 1

--INPUT--
<?php
// $Id: tags.phpt,v 1.2 2008/02/24 19:40:50 sun Exp $

function ugly_foo() {
  $bar = bar();
  ?>
  <div class="foo">
    <?php print $bar; ?>
  </div>
  <?php
  return $baz;
}

--INPUT--
<?php
// $Id: tags.phpt,v 1.2 2008/02/24 19:40:50 sun Exp $

function l() {
  ?>
  <a href="<?php print $link; ?>"><?php print $title; ?></a>
  <?php
}

--INPUT--
<?php
// $Id: tags.phpt,v 1.2 2008/02/24 19:40:50 sun Exp $


/**
 * Theme a node.
 */
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?> clear-block">

<?php print $picture ?>

<?php if (!$page): ?>
  <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
<?php endif;

