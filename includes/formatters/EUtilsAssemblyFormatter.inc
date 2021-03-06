<?php

/**
 * Parse EUtilsAssemblyParser output for display on a form.
 */
class EUtilsAssemblyFormatter extends EUtilsFormatter {

  private $elements = [];

  /**
   * Add the formatted data into a table.
   *
   * @param array $data
   *   The parsed XML data.
   *
   * @return array
   *   Drupal form elements array of each section in a fieldset.
   */
  public function format(array $data) {
    try {
      $this->formatBaseRecord($data);
      $this->formatAttributes($data['attributes']);
      $this->formatXrefs($data['accessions']['assembly']);
      unset($data['accessions']['assembly']);
      $this->formatLinkedRecords($data['accessions']);
    }
    catch (Exception $exception) {
      drupal_set_message($exception->getMessage());
      return NULL;
    }

    return $this->elements;
  }

  /**
   * Handle the base Chado.analysis record.
   *
   * @param array $data
   *   From \EutilsAssemblyParser.
   */
  private function formatBaseRecord(array $data) {
    if (!isset($data['name'])) {
      throw new Exception("Invalid record format.");
    }
    $name = $data['name'];
    $description = $data['description'];
    $program = $data['attributes']['ftp_attributes']['# Assembly method:'] ?? "Not set";
    $sourcename = $data['sourcename'];

    $type = 'Genome Assembly';
    $xrefs = $data['accessions']['assembly'];
    $xref_string = $this->generateXrefString($xrefs);
    $header = [
      'Name',
      'Description',
      'Source Name',
      'Type',
      'Program',
      'Accessions',
    ];
    $rows = [[$name, $description, $sourcename, $type, $program, $xref_string]];
    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['base_record'] = [
      '#type' => 'fieldset',
      '#title' => 'Analysis Record',
      '#collapsible' => TRUE,
    ];
    $this->elements['base_record']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];
  }

  /**
   * Describe the Chado properties that will be associated with this analysis.
   *
   * @param array $attributes
   *   Array of attributes.  Should have stats and file keys.
   */
  private function formatAttributes(array $attributes) {
    $header = ['Property Name', 'Value'];
    $rows = [];
    foreach ($attributes['stats'] as $key => $value) {
      $rows[] = [$key, $value];
    }
    foreach ($attributes['files'] as $key => $value) {
      $rows[] = [$key, $value];
    }

    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['properties'] = [
      '#type' => 'fieldset',
      '#title' => 'Properties',
      '#collapsible' => TRUE,
    ];
    $this->elements['properties']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];
  }

  /**
   * Describes additional records that will be created.
   *
   * @param array $accessions
   *   the Accessions array from the EutilsAssemblyParser.
   */
  private function formatLinkedRecords(array $accessions) {
    $bioproject_found = FALSE;
    $biosample_found = FALSE;
    $rows = [];
    $header = ['Accession Type', 'Value'];

    $db = NULL;
    foreach ($accessions as $accession_key => $value) {
      $db_name = '';

      if ($accession_key == 'assembly') {
        // This is the record itself.
        $db_name = 'NCBI Assembly';
      }
      if ($accession_key == 'taxon_accession') {

        $link = $this->getDbLink($value, 'NCBITaxon');
        $rows[] = ['Organism', $link];
        continue;
      }
      if ($accession_key == 'bioprojects') {
        $db_name = 'BioProject';
        $bioproject_found = TRUE;
      }
      if ($accession_key == 'biosamples') {
        $db_name = 'BioSample';
        $biosample_found = TRUE;
      }

      if (is_string($value)) {
        $value = $this->getDbLink($value, $db_name);
        $rows[] = [ucfirst($accession_key), $value];
        continue;
      }

      if ($db_name == 'NCBI Assembly') {

        // Convert any accessions to integer to remove duplicates.
        $unique = [];

        $eutils = new EUtils(FALSE);
        foreach ($value as $accession) {
          $accession = $eutils->convertAccessionsToUID('Assembly', $accession);
          $unique[] = $accession;
        }
        $value = $unique;
      }
      foreach ($value as $key => $item) {
        $link = $this->getDbLink($item, $db_name);
        $rows[] = [ucfirst($accession_key), $link];
      }
    }

    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['links'] = [
      '#type' => 'fieldset',
      '#title' => 'Additional Records',
      '#collapsible' => TRUE,
    ];
    $this->elements['links']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];

    // We are going to link biosample indirectly via project.
    if ($biosample_found && !$bioproject_found) {
      tripal_set_message('A bioMaterial record is linked, but no BioSample.  Please import the BioSample directly.', TRIPAL_NOTICE);
    }
  }


  private function formatXrefs(array $accessions) {

    $rows =[];
    $header = ['Database', 'Accession'];
    foreach ($accessions as $key => $item) {

      $link = $this->getDbLink($item, $key);
      $rows[] = [$key, $link];
    }

    if (empty($rows)) {
      return;
    }

    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['xrefs'] = [
      '#type' => 'fieldset',
      '#title' => 'Cross References',
      '#collapsible' => TRUE,
    ];
    $this->elements['xrefs']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];


  }


  /**
   * Generates a cross reference string.  Allows us to put multiple xrefs in one row.
   *
   * @param array $xrefs
   *   Cross reference array.
   *
   * @return string
   *   A bunch of xrefs with links in a single string.
   */
  private function generateXrefString(array $xrefs) {
    if (empty($xrefs)) {
      return '';
    }

    $links = [];
    foreach ($xrefs as $db => $xref) {

      $link = $this->getDbLink($xref, 'assembly');
      $link = $db . ':' . $link;
      $links[] = $link;
    }

    return implode(', ', $links);
  }

}
