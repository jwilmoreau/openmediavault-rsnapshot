<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceRSnapshot extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "RSnapshot";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get list of USB backup job configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getList($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Get the configuration objects.
		$xpath = "//services/rsnapshot/jobs/job";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		foreach($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object.
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if(!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
			// Get job status.
			$objectv['running'] = file_exists(sprintf("/var/run/sync-%s",
			  $objectv['fsuuid']));
			// Modify result object.
			$objectv['enable'] = boolval($objectv['enable']);
			$objectv['sendemail'] = boolval($objectv['sendemail']);
			$objectv['usesubdir'] = boolval($objectv['usesubdir']);
			$objectv['recursive'] = boolval($objectv['recursive']);
			$objectv['times'] = boolval($objectv['times']);
			$objectv['compress'] = boolval($objectv['compress']);
			$objectv['archive'] = boolval($objectv['archive']);
			$objectv['delete'] = boolval($objectv['delete']);
			$objectv['quiet'] = boolval($objectv['quiet']);
			$objectv['perms'] = boolval($objectv['perms']);
			$objectv['acls'] = boolval($objectv['acls']);
			$objectv['xattrs'] = boolval($objectv['xattrs']);
		}
		// Filter the result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a USB backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//services/rsnapshot/jobs/job[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
		$object['enable'] = boolval($object['enable']);
		$object['sendemail'] = boolval($object['sendemail']);
		$object['usesubdir'] = boolval($object['usesubdir']);
		$object['recursive'] = boolval($object['recursive']);
		$object['times'] = boolval($object['times']);
		$object['compress'] = boolval($object['compress']);
		$object['archive'] = boolval($object['archive']);
		$object['delete'] = boolval($object['delete']);
		$object['quiet'] = boolval($object['quiet']);
		$object['perms'] = boolval($object['perms']);
		$object['acls'] = boolval($object['acls']);
		$object['xattrs'] = boolval($object['xattrs']);
		$object['partial'] = boolval($object['partial']);
		return $object;
	}

	/**
	 * Set a USB backup job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function set($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "enable":{"type":"boolean"},
				  "mode":{"type":"string","enum":["push","pull"]},
				  "sendemail":{"type":"boolean"},
				  "comment":{"type":"string"},
				  "sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "usesubdir":{"type":"boolean"},
				  "recursive":{"type":"boolean"},
				  "times":{"type":"boolean"},
				  "compress":{"type":"boolean"},
				  "archive":{"type":"boolean"},
				  "delete":{"type":"boolean"},
				  "quiet":{"type":"boolean"},
				  "perms":{"type":"boolean"},
				  "acls":{"type":"boolean"},
				  "xattrs":{"type":"boolean"},
				  "partial":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"enable" => array_boolval($params, 'enable'),
			"mode" => $params['mode'],
			"sendemail" => array_boolval($params, 'sendemail'),
			"comment" => $params['comment'],
			"sharedfolderref" => $params['sharedfolderref'],
			"usesubdir" => array_boolval($params, 'usesubdir'),
			"recursive" => array_boolval($params, 'recursive'),
			"times" => array_boolval($params, 'times'),
			"compress" => array_boolval($params, 'compress'),
			"archive" => array_boolval($params, 'archive'),
			"delete" => array_boolval($params, 'delete'),
			"quiet" => array_boolval($params, 'quiet'),
			"perms" => array_boolval($params, 'perms'),
			"acls" => array_boolval($params, 'acls'),
			"xattrs" => array_boolval($params, 'xattrs'),
			"partial" => array_boolval($params, 'partial'),
			"extraoptions" => $params['extraoptions']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/rsnapshot/jobs",
			  array("job" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/rsnapshot/jobs/job[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.rsnapshot.jobs.job", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//services/rsnapshot/jobs/job[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.rsnapshot.jobs.job");
	}

	/**
	 * Execute a backup job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the cron job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_CONFIG_DIRTY
	 */
	public function execute($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron script has not been created or updated
		// until now.
		if($this->isModuleDirty("rsnapshot")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);
		}
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Execute the backup script.
			$cmd = "/bin/sh";
			$cmdArgs = array(sprintf("%s/rsync-%s",
			  $GLOBALS['OMV_RSNAPSHOT_SCRIPTS_DIR'], $params['uuid']));
			if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd." ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRSnapshot());
?>