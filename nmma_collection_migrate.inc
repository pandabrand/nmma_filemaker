<?php

class NMMACollectionMigration extends Migration {
  public function __construct() {
    parent::__construct($arguments);

    $this->description = t('Import Collection from FileMaker Cloud');
    $this->defaultLanguage = 'en';

		$fields = array(
			'recordId' => t('Record ID'),
			'Approved_for_Web' => t('Approved for Web'),
			'Main' => t('Main'),
			'Image_Ready' => t('Image ready'),
			'Artist_Birth' => t('Artist Birth'),
			'Artist_Death' => t('Artist Death'),
			'Artist_Country' => t('Artist Country'),
			'Accession_number' => t('Accession Number'),
			'Accession_Part' => t('Accession part'),
			'Object_Type' => t('Object Type'),
			'Culture' => t('Culture'),
			'Artist_First_Name' => t('Artist First Name'),
			'Artist_Middle_Name' => t('Artist Middle Name'),
			'Artist_Last_Name' => t('Artist Last Name'),
			'Designer_Last_Name' => t('Designer Last Name'),
			'Designer_First_Name' => t('Designer First Name'),
			'Designer_Middle_Name' => t('Designer Middle Name'),
			'Creation_Month' => t('Creation Month'),
			'Photographer_First_Name' => t('Photographer First Name'),
			'Photographer_Last_Name' => t('Photographer Last Name'),
			'Photographer_Middle_Name' => t('Photographer Middle Name'),
			'Title' => t('Title'),
			'Phase_Style' => t('Phase Style'),
			'Translation' => t('Translation'),
			'Period' => t('Period'),
			'Region' => t('Region'),
			'Media' => t('Media'),
			'Edition_Run' => t('Edition Run'),
			'Creation_Year' => t('Creation Year'),
			'Dimensions' => t('Dimensions'),
			'Paper_Dimensions' => t('Paper Dimensions'),
			'Credit' => t('Credit'),
      'Image Name' => t('Image Name'),
      'permcoll_IMAGES~recordID::webImage' => t('Web Image Path'),
      'permcoll_IMAGES::webImageName' => t('Web Image Name'),
    );

		$json_file = '/example/data/example-response.json';

    $this->source = new MigrateSourceList(new FileMakerListJSON($json_file), new FileMakerItemJSON($json_file, array(), $fields));
    $this->destination = new MigrateDestinationNode('nmma_collection_item_json');

		// Mapping

		$keySchema = MigrateDestinationNode::getKeySchema();

		$this->map = new MigrateSQLMap($this->machineName,
			array(
				'recordId' => array(
					'type' => 'varchar',
					'length' => 255,
				),
			),
			$keySchema
		);

  }
}