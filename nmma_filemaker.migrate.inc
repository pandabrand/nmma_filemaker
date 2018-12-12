<?php

/**
 * @file
 * Declares our migrations.
 */


/**
 * Implements hook_migrate_api().
 */
function nmma_filemaker_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'nmma_filemaker' => array(
        'title' => t('NMMA Filemaker Migrations'),
      ),
    ),
    'migrations' => array(
      'NMMACollectionItems' => array(
        'class_name' => 'NMMACollectionMigration',
        'group_name' => 'nmma_filemaker',
      ),
    ),
  );
  return $api;
}