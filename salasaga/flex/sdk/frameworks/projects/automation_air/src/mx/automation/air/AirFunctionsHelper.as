////////////////////////////////////////////////////////////////////////////////
//
//  ADOBE SYSTEMS INCORPORATED
//  Copyright 2009 Adobe Systems Incorporated
//  All Rights Reserved.
//
//  NOTICE: Adobe permits you to use, modify, and distribute this file
//  in accordance with the terms of the license agreement accompanying it.
//
////////////////////////////////////////////////////////////////////////////////

package mx.automation.air
{
	import flash.display.DisplayObject;
	import flash.geom.Point;
	
	import mx.automation.Automation;
	import mx.automation.IAutomationManager2;
	import mx.core.FlexGlobals;
	[Mixin] 
	public class AirFunctionsHelper
	{
		private var _stageStartCoordinates:Point;
		//private var _stageWidth:int;
		//private var _stageHeight:int;
		
		//private var _applicationStartCoordinates:Point;
		//private var _applicationWidth:int;
		//private var _applicationHeight:int;
		
		
		//private var _chromeHeight:int;	
		//private var _chromeWidth:int;
		
		private var _appTitle:String;
		private var _appWindow:DisplayObject; 
		
		public function AirFunctionsHelper(windowId:String)
		{
			if (!windowId)
				//_appWindow =   Application.application as DisplayObject;
				_appWindow =   FlexGlobals.topLevelApplication as DisplayObject;
			else
			{
				var automationManager:IAutomationManager2 = Automation.automationManager2;
				if(automationManager)
					_appWindow = automationManager.getAIRWindow(windowId);
			}
		}
		
		public static function init(root:DisplayObject):void
		{
			
		}
		
		// native window 's bound will give the totla and stage gives inner (statge width and stage heigjt
		
		public  function get stageStartCoordinates():Point
		{
			// every time when the user asks this, we need to recalcuate and send the same
			
			var stageStartScreenCoordinates:Point;
			
			var startPoint:Point = new Point(0,0);
			if (_appWindow)
			{
				var startGloabalPoint:Point = _appWindow.localToGlobal(startPoint);
				//stageStartScreenCoordinates = Application.application.stage.nativeWindow.globalToScreen(startGloabalPoint);
				stageStartScreenCoordinates = _appWindow.stage.nativeWindow.globalToScreen(startGloabalPoint);
			}
			
			_stageStartCoordinates = stageStartScreenCoordinates;
			return _stageStartCoordinates;
			
		}
		
		public function get activeWindow():DisplayObject
		{
			if(_appWindow != null)
				return _appWindow;
			else
				return FlexGlobals.topLevelApplication as DisplayObject;
		}
		
		public  function get appTitle():String
		{
			_appTitle=   FlexGlobals.topLevelApplication.stage.nativeWindow.title;
			return _appTitle;
		}
	}
}