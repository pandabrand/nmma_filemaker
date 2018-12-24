<?php

class NMMACollectionMigration extends Migration {
  public function __construct($arguments) {
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

		$json_folder = DRUPAL_ROOT . '/' . 'sites/all/modules/nmma_filemaker/example/data/';
		$json_file = $json_folder . 'example-response.json';

		$this->source = new MigrateSourceList(new FileMakerListJSON($json_file), new FileMakerItemJSON($json_file, array()), $fields);
		// $this->source = new MigrateSourceJSON($json_file, 'recordId', $fields, array('reader_class' => 'FileMakerJSONReader'));
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

		$this->addFieldMapping('uid')->defaultValue(100223);
		$this->addFieldMapping('title', 'Title')->callbacks(array($this, 'trimTitle'));
		$this->addFieldMapping('field_full_title', 'Title');
		$this->addFieldMapping('field_accession_number', 'Accession_number');
		$this->addFieldMapping('field_accession_part', 'Accession_Part');
		$this->addFieldMapping('field_approved_for_web', 'Approved_for_Web')->callbacks(array($this, 'computeApprovedForWeb'));
		$this->addFieldMapping('field_image_ready', 'Image_Ready')->callbacks(array($this, 'computeImageReady'));
		$this->addFieldMapping('field_artist_first_name', 'Artist_First_Name');
		$this->addFieldMapping('field_artist_middle_name', 'Artist_Middle_Name');
		$this->addFieldMapping('field_artist_last_name', 'Artist_Last_Name');
		$this->addFieldMapping('field_designer_first_name', 'Designer_First_Name');
		$this->addFieldMapping('field_designer_middle_name', 'Designer_Middle_Name');
		$this->addFieldMapping('field_designer_last_name', 'Designer_Last_Name');
		$this->addFieldMapping('field_photographer_first_name', 'Photographer_First_Name');
		$this->addFieldMapping('field_photographer_middle_name', 'Photographer_Middle_Name');
		$this->addFieldMapping('field_photographer_last_name', 'Photographer_Last_Name');
		$this->addFieldMapping('field_artist_birth', 'Artist_Birth');
		$this->addFieldMapping('field_artist_death', 'Artist_Death');
		$this->addFieldMapping('field_artist_country', 'Artist_Country');
		$this->addFieldMapping('field_culture', 'Culture');
		$this->addFieldMapping('field_culture:create_term')->defaultValue(TRUE);
		$this->addFieldMapping('field_culture:ignore_case')->defaultValue(TRUE);
		$this->addFieldMapping('field_object_type', 'Object_Type');
		$this->addFieldMapping('field_phase_style', 'Phase_Style');
		$this->addFieldMapping('field_period', 'Period');
		$this->addFieldMapping('field_period:create_term')->defaultValue(TRUE);
		$this->addFieldMapping('field_period:ignore_case')->defaultValue(TRUE);
		$this->addFieldMapping('field_creation_year', 'Creation_Year');
		$this->addFieldMapping('field_dimensions', 'Dimensions');
		$this->addFieldMapping('field_paper_dimensions', 'Paper_Dimensions');
		$this->addFieldMapping('field_edition_run', 'Edition_Run');
		$this->addFieldMapping('field_credit', 'Credit');
		$this->addFieldMapping('field_region', 'Region');
		$this->addFieldMapping('field_region:create_term');
		$this->addFieldMapping('field_region:ignore_case');
		$this->addFieldMapping('field_translation', 'Translation');
		$this->addFieldMapping('field_media', 'Media');
  }
}