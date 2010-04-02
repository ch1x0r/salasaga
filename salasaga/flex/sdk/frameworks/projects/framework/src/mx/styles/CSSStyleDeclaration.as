////////////////////////////////////////////////////////////////////////////////
//
//  ADOBE SYSTEMS INCORPORATED
//  Copyright 2003-2007 Adobe Systems Incorporated
//  All Rights Reserved.
//
//  NOTICE: Adobe permits you to use, modify, and distribute this file
//  in accordance with the terms of the license agreement accompanying it.
//
////////////////////////////////////////////////////////////////////////////////

package mx.styles
{

import flash.display.DisplayObject;
import flash.events.EventDispatcher;
import flash.utils.Dictionary;

import mx.core.Singleton;
import mx.core.mx_internal;
import mx.events.FlexChangeEvent;
import mx.managers.ISystemManager;
import mx.managers.SystemManagerGlobals;
import mx.utils.ObjectUtil;

use namespace mx_internal;

/**
 *  The CSSStyleDeclaration class represents a set of CSS style rules.
 *  The MXML compiler automatically generates one CSSStyleDeclaration object
 *  for each selector in the CSS files associated with a Flex application.
 *  
 *  <p>A CSS rule such as
 *  <pre>
 *      Button { color: #FF0000 }
 *  </pre>
 *  affects every instance of the Button class;
 *  a selector like <code>Button</code> is called a type selector
 *  and must not start with a dot.</p>
 *
 *  <p>A CSS rule such as
 *  <pre>
 *      .redButton { color: #FF0000 }
 *  </pre>
 *  affects only components whose <code>styleName</code> property
 *  is set to <code>"redButton"</code>;
 *  a selector like <code>.redButton</code> is called a class selector
 *  and must start with a dot.</p>
 *
 *  <p>You can access the autogenerated CSSStyleDeclaration objects
 *  using the <code>StyleManager.getStyleDeclaration()</code> method,
 *  passing it either a type selector
 *  <pre>
 *  var buttonDeclaration:CSSStyleDeclaration =
 *      StyleManager.getStyleDeclaration("Button");
 *  </pre>
 *  or a class selector
 *  <pre>
 *  var redButtonStyleDeclaration:CSSStyleDeclaration =
 *      StyleManager.getStyleDeclaration(".redButton");
 *  </pre>
 *  </p>
 *
 *  <p>You can use the <code>getStyle()</code>, <code>setStyle()</code>,
 *  and <code>clearStyle()</code> methods to get, set, and clear 
 *  style properties on a CSSStyleDeclaration.</p>
 *
 *  <p>You can also create and install a CSSStyleDeclaration at run time
 *  using the <code>StyleManager.setStyleDeclaration()</code> method:
 *  <pre>
 *  var newStyleDeclaration:CSSStyleDeclaration = new CSSStyleDeclaration(".bigMargins");
 *  newStyleDeclaration.defaultFactory = function():void
 *  {
 *      leftMargin = 50;
 *      rightMargin = 50;
 *  }
 *  StyleManager.setStyleDeclaration(".bigMargins", newStyleDeclaration, true);
 *  </pre>
 *  </p>
 *
 *  @see mx.core.UIComponent
 *  @see mx.styles.StyleManager
 *  
 *  @langversion 3.0
 *  @playerversion Flash 9
 *  @playerversion AIR 1.1
 *  @productversion Flex 3
 */
public class CSSStyleDeclaration extends EventDispatcher
{
    include "../core/Version.as";

    //--------------------------------------------------------------------------
    //
    //  Class constants
    //
    //--------------------------------------------------------------------------

    /**
     *  @private
     */
    private static const NOT_A_COLOR:uint = 0xFFFFFFFF;

    /**
     *  @private
     */
    private static const FILTERMAP_PROP:String = "__reserved__filterMap";
    
    //--------------------------------------------------------------------------
    //
    //  Constructor
    //
    //--------------------------------------------------------------------------

    /**
     *  Constructor.
     *
     *  @param selector - If the selector is a CSSSelector then advanced
     *  CSS selectors are supported. If a String is used for the selector then
     *  only simple CSS selectors are supported. If the String starts with a
     *  dot it is interpreted as a universal class selector, otherwise it must
     *  represent a simple type selector. If not null, this CSSStyleDeclaration
     *  will be registered with StyleManager. 
     *  
     *  @param styleManager - The style manager to set this declaration into. If the
     *  styleManager is null the top-level style manager will be used.
     * 
     *  @param autoRegisterWithStyleManager - If true set the selector in the styleManager. The selector
     *  will only be set if both <code>selector</code> and <code>styleManager</code> are
     *  both non-null.
     *
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function CSSStyleDeclaration(selector:Object=null, styleManager:IStyleManager2=null, autoRegisterWithStyleManager:Boolean = true)
    {
        super();

        // Do not reference StyleManager directly because this is a bootstrap class
        if (!styleManager)
            styleManager = Singleton.getInstance("mx.styles::IStyleManager2") as IStyleManager2;

        this.styleManager = styleManager;

        if (selector)
        {
            if (selector is CSSSelector)
            {
                this.selector = selector as CSSSelector;
            }
            else
            {
                // Otherwise, a legacy Flex 3 String selector was provided
                selectorString = selector.toString();
            }

            if (autoRegisterWithStyleManager)
                styleManager.setStyleDeclaration(selectorString, this, false);            
        }

    }

    //--------------------------------------------------------------------------
    //
    //  Variables
    //
    //--------------------------------------------------------------------------

    /**
     *  @private
     *  This Dictionary keeps track of all the style name/value objects
     *  produced from this CSSStyleDeclaration and already inserted into
     *  prototype chains. Whenever this CSSStyleDeclaration's overrides object
     *  is updated by setStyle(), these clone objects must also be updated.
     */
    private var clones:Dictionary = new Dictionary(true);

    /**
     *  @private
     *  The number of CSS selectors pointing to this CSSStyleDeclaration.
     *  It will be greater than 0 if this CSSStyleDeclaration has been
     *  installed in the StyleManager.styles table by
     *  StyleManager.setStyleDeclaration().
     */
    mx_internal var selectorRefCount:int = 0;

    /**
     *  @private
     *  Array that specifies the names of the events declared
     *  by this CSS style declaration.
     *  This Array is used by the <code>StyleProtoChain.initObject()</code>
     *  method to register the effect events with the Effect manager.
     */
    mx_internal var effects:Array;

    /**
     *  @private
     *  reference to StyleManager
     */
    private var styleManager:IStyleManager2;

    //--------------------------------------------------------------------------
    //
    //  Properties
    //
    //--------------------------------------------------------------------------
    
    //----------------------------------
    //  defaultFactory
    //----------------------------------

    private var _defaultFactory:Function;
    
    [Inspectable(environment="none")]
    
    /**
     *  This function, if it isn't <code>null</code>,
     *  is usually autogenerated by the MXML compiler.
     *  It produce copies of a plain Object, such as
     *  <code>{ leftMargin: 10, rightMargin: 10 }</code>,
     *  containing name/value pairs for style properties; the object is used
     *  to build a node of the prototype chain for looking up style properties.
     *
     *  <p>If this CSSStyleDeclaration is owned by a UIComponent
     *  written in MXML, this function encodes the style attributes
     *  that were specified on the root tag of the component definition.</p>
     *
     *  <p>If the UIComponent was written in ActionScript,
     *  this property is <code>null</code>.</p>
     *  
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function get defaultFactory():Function
    {
        return _defaultFactory;
    }
    
    /**
     *  @private
     */ 
    public function set defaultFactory(f:Function):void
    {
        _defaultFactory = f;
    }
    
    //----------------------------------
    //  factory
    //----------------------------------

    private var _factory:Function;

    [Inspectable(environment="none")]
    
    /**
     *  This function, if it isn't <code>null</code>,
     *  is usually autogenerated by the MXML compiler.
     *  It produce copies of a plain Object, such as
     *  <code>{ leftMargin: 10, rightMargin: 10 }</code>,
     *  containing name/value pairs for style properties; the object is used
     *  to build a node of the prototype chain for looking up style properties.
     *
     *  <p>If this CSSStyleDeclaration is owned by a UIComponent,
     *  this function encodes the style attributes that were specified in MXML
     *  for an instance of that component.</p>
     *  
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function get factory():Function
    {
        return _factory;
    }
    
    /**
     *  @private
     */ 
    public function set factory(f:Function):void
    {
        _factory = f;
    }

    //----------------------------------
    //  overrides
    //----------------------------------

    private var _overrides:Object;

    /**
     *  If the <code>setStyle()</code> method is called on a UIComponent or CSSStyleDeclaration
     *  at run time, this object stores the name/value pairs that were set;
     *  they override the name/value pairs in the objects produced by
     *  the  methods specified by the <code>defaultFactory</code> and 
     *  <code>factory</code> properties.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function get overrides():Object
    {
        return _overrides;
    }
    
    /**
     *  @private
     */ 
    public function set overrides(o:Object):void
    {
        _overrides = o;
    }
    
    //----------------------------------
    //  selector
    //----------------------------------

    private var _selector:CSSSelector;

    /**
     *  This property is the base selector of a potential chain of selectors
     *  and conditions that are used to match CSS style declarations to
     *  components.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 10
     *  @playerversion AIR 1.5
     *  @productversion Flex 4
     */
    public function get selector():CSSSelector
    {
        return _selector; 
    }

    public function set selector(value:CSSSelector):void
    {
        _selector = value;
        _selectorString = null;
    }

    //----------------------------------
    //  selectorString
    //----------------------------------

    private var _selectorString:String;

    /**
     *  Legacy support for setting a Flex 3 styled selector string after 
     *  the construction of a style declaration. Only universal class selectors
     *  or simple type selectors are supported. Note that this style declaration
     *  is not automatically registered with the StyleManager when using this
     *  API.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 10
     *  @playerversion AIR 1.5
     *  @productversion Flex 4
     */
    mx_internal function get selectorString():String
    {
        if (_selectorString == null && _selector != null)
            _selectorString = _selector.toString();

        return _selectorString; 
    }

    mx_internal function set selectorString(value:String):void
    {
        // For the legacy API, the first argument is either a simple
        // type selector or a universal class selector
        if (value.charAt(0) == ".")
        {
            var condition:CSSCondition = new CSSCondition(CSSConditionKind.CLASS, value.substr(1));
            _selector = new CSSSelector("", [condition]);
        }
        else
        {
            _selector = new CSSSelector(value);
        }

        _selectorString = value;
    }

    //----------------------------------
    //  specificity
    //----------------------------------

    /**
     *  Determines the order of precedence when applying multiple style
     *  declarations to a component. If style declarations are of equal
     *  precedence, the last one wins. 
     *  
     *  @langversion 3.0
     *  @playerversion Flash 10
     *  @playerversion AIR 1.5
     *  @productversion Flex 4
     */
    public function get specificity():int
    {
        return _selector ? _selector.specificity : 0; 
    }

    //----------------------------------
    //  subject
    //----------------------------------

    /**
     *  The subject describes the name of a component that may be a potential
     *  match for this style declaration. The subject is determined as right
     *  most simple type selector in a potential chain of selectors.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 10
     *  @playerversion AIR 1.5
     *  @productversion Flex 4
     */
    public function get subject():String
    {
        if (_selector != null)
        {
            // Check for an implicit universal selector which omits *
            // for the subject but includes conditions.
            if (_selector.subject == "" && _selector.conditions)
                return "*";
            else
                return _selector.subject;
        }

        return null;
    }

    //--------------------------------------------------------------------------
    //
    //  Methods
    //
    //--------------------------------------------------------------------------

    /**
     *  @private
     *  Determines whether the selector chain for this style declaration makes
     *  use of a pseudo condition.
     */  
    mx_internal function getPseudoCondition():String
    {
        return (selector != null) ? selector.getPseudoCondition() : null;
    }

    /**
     * @private
     * Determines whether this style declaration has an advanced selector.
     */  
    mx_internal function isAdvanced():Boolean
    {
        if (selector != null)
        {
            if (selector.ancestor)
            {
                return true;
            }
            else if (selector.conditions)
            {
                if (subject != "*" && subject != "global")
                {
                    return true;
                }

                for each (var condition:CSSCondition in selector.conditions)
                {
                    if (condition.kind != CSSConditionKind.CLASS)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /**
     *  Determines whether this style declaration applies to the given component
     *  based on a match of the selector chain.
     * 
     *  @return true if this style declaration applies to the component, 
     *  otherwise false. 
     *  
     *  @langversion 3.0
     *  @playerversion Flash 10
     *  @playerversion AIR 1.5
     *  @productversion Flex 4
     */
    public function matchesStyleClient(object:IAdvancedStyleClient):Boolean
    {
        return (selector != null) ? selector.matchesStyleClient(object) : false;
    }

    /**
     *  Determine if the properties of this style declaration are the same as the the properties of a specified
     *  style declaration.
     * 
     *  @param styleDeclaration the style declaration to compare.
     * 
     *  @return true if the styleDeclaration is considered equal to this declaration. 
     */ 
    mx_internal function equals(styleDeclaration:CSSStyleDeclaration):Boolean
    {
        if (styleDeclaration == null)
            return false;
        
        // test in order of most likey to be different.
        
        var obj:Object; // loop variable
        
        // overrides
        if (ObjectUtil.compare(overrides, styleDeclaration.overrides) != 0)
            return false;

        // factory
        if ((factory == null && styleDeclaration.factory != null) ||
            (factory != null && styleDeclaration.factory == null))
            return false;

        if (factory != null)
        {
            if (ObjectUtil.compare(new factory(), new styleDeclaration.factory()) != 0)
                return false;
        }
        
        // defaultFactory
        if ((defaultFactory == null && styleDeclaration.defaultFactory != null) ||
            (defaultFactory != null && styleDeclaration.defaultFactory == null))
            return false;
        
        if (defaultFactory != null)
        {
            if (ObjectUtil.compare(new defaultFactory(), 
                                   new styleDeclaration.defaultFactory()) != 0)
                return false;
        }
        
        // effects
        if (ObjectUtil.compare(effects, styleDeclaration.mx_internal::effects))
            return false;
                
        return true;
    }
    

    /**
     *  Gets the value for a specified style property,
     *  as determined solely by this CSSStyleDeclaration.
     *
     *  <p>The returned value may be of any type.</p>
     *
     *  <p>The values <code>null</code>, <code>""</code>, <code>false</code>,
     *  <code>NaN</code>, and <code>0</code> are all valid style values,
     *  but the value <code>undefined</code> is not; it indicates that
     *  the specified style is not set on this CSSStyleDeclaration.
     *  You can use the method <code>StyleManager.isValidStyleValue()</code>
     *  to test the value that is returned.</p>
     *
     *  @param styleProp The name of the style property.
     *
     *  @return The value of the specified style property if set,
     *  or <code>undefined</code> if not.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function getStyle(styleProp:String):*
    {
        var o:*;
        var v:*;

        // First look in the overrides, in case setStyle()
        // has been called on this CSSStyleDeclaration.
        if (overrides)
        {
            // If the property exists in our overrides, but 
            // has 'undefined' as its value, it has been 
            // cleared from this stylesheet so return
            // undefined.
            if (styleProp in overrides &&
                overrides[styleProp] === undefined)
                return undefined;
                
            v = overrides[styleProp];
            if (v !== undefined) // must use !==
                return v;
        }

        // Next look in the style object that this CSSStyleDeclaration's
        // factory function produces; it contains styles that
        // were specified in an instance tag of an MXML component
        // (if this CSSStyleDeclaration is attached to a UIComponent).
        if (factory != null)
        {
            factory.prototype = {};
            o = new factory();
            v = o[styleProp];
            if (v !== undefined) // must use !==
                return v;
        }

        // Next look in the style object that this CSSStyleDeclaration's
        // defaultFactory function produces; it contains styles that
        // were specified on the root tag of an MXML component.
        if (defaultFactory != null)
        {
            defaultFactory.prototype = {};
            o = new defaultFactory();
            v = o[styleProp];
            if (v !== undefined) // must use !==
                return v;
        }

        // Return undefined if the style isn't specified
        // in any of these three places.
        return undefined;
    }

    /**
     *  Sets a style property on this CSSStyleDeclaration.
     *
     *  @param styleProp The name of the style property.
     *
     *  @param newValue The value of the style property.
     *  The value may be of any type.
     *  The values <code>null</code>, <code>""</code>, <code>false</code>,
     *  <code>NaN</code>, and <code>0</code> are all valid style values,
     *  but the value <code>undefined</code> is not.
     *  Setting a style property to the value <code>undefined</code>
     *  is the same as calling the <code>clearStyle()</code> method.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function setStyle(styleProp:String, newValue:*):void
    {
        var oldValue:Object = getStyle(styleProp);
        var regenerate:Boolean = false;

        // If this CSSStyleDeclaration didn't previously have a factory,
        // defaultFactory, or overrides object, then this CSSStyleDeclaration
        // hasn't been added to anyone's proto chain.  In that case, we
        // need to regenerate everyone's proto chain.
        if (selectorRefCount > 0 &&
            factory == null &&
            defaultFactory == null &&
            !overrides && 
            (oldValue !== newValue)) // must be !==
        {
            regenerate = true;
        }
        
        if (newValue !== undefined) // must be !==
        {
            setLocalStyle(styleProp, newValue);
        }
        else
        {
            if (newValue == oldValue)
                return;
            setLocalStyle(styleProp, newValue);
        }

        var sms:Array = SystemManagerGlobals.topLevelSystemManagers;
        var n:int = sms.length;
        var i:int;

        // Type as Object to avoid dependency on SystemManager.
        var sm:ISystemManager;
        var cm:Object;

        if (regenerate)
        {
            // Regenerate all the proto chains
            // for all objects in the application.
            for (i = 0; i < n; i++)
            {
                sm = sms[i];
			    cm = sm.getImplementation("mx.managers::ISystemManagerChildManager");
                cm.regenerateStyleCache(true);
            }
        }

        for (i = 0; i < n; i++)
        {
            sm = sms[i];
		    cm = sm.getImplementation("mx.managers::ISystemManagerChildManager");
            cm.notifyStyleChangeInChildren(styleProp, true);
        }
    }
    
    /**
     *  @private
     *  Sets a style property on this CSSStyleDeclaration.
     *
     *  @param styleProp The name of the style property.
     *
     *  @param newValue The value of the style property.
     *  The value may be of any type.
     *  The values <code>null</code>, <code>""</code>, <code>false</code>,
     *  <code>NaN</code>, and <code>0</code> are all valid style values,
     *  but the value <code>undefined</code> is not.
     *  Setting a style property to the value <code>undefined</code>
     *  is the same as calling <code>clearStyle()</code>.
     */
    mx_internal function setLocalStyle(styleProp:String, value:*):void
    {
        var oldValue:Object = getStyle(styleProp);

        // If setting to undefined, clear the style attribute.
        if (value === undefined) // must use ===
        {
            clearStyleAttr(styleProp);
            return;
        }

        var o:Object;

        // If the value is a String of the form "#FFFFFF" or "red",
        // then convert it to a RGB color uint (e.g.: 0xFFFFFF).
        if (value is String)
        {
            if (!styleManager)
                styleManager = Singleton.getInstance("mx.styles::IStyleManager2") as IStyleManager2;
            var colorNumber:Number = styleManager.getColorName(value);
            if (colorNumber != NOT_A_COLOR)
                value = colorNumber;
        }

        // If the new value for styleProp is different from the one returned
        // from the defaultFactory function, then store the new value on the
        // overrides object. That way, future clones will get the new value.
        if (defaultFactory != null)
        {
            o = new defaultFactory();
            if (o[styleProp] !== value) // must use !==
            {
                if (!overrides)
                    overrides = {};
                overrides[styleProp] = value;
            }
            else if (overrides)
            {
                delete overrides[styleProp];
            }
        }

        // If the new value for styleProp is different from the one returned
        // from the factory function, then store the new value on the
        // overrides object. That way, future clones will get the new value.
        if (factory != null)
        {
            o = new factory();
            if (o[styleProp] !== value) // must use !==
            {
                if (!overrides)
                    overrides = {};
                overrides[styleProp] = value;
            }
            else if (overrides)
            {
                delete overrides[styleProp];
            }
        }

        if (defaultFactory == null && factory == null)
        {
            if (!overrides)
                overrides = {};
            overrides[styleProp] = value;
        }

        // Update all clones of this style sheet.
        updateClones(styleProp, value);
        
    }
    
    /**
     *  Clears a style property on this CSSStyleDeclaration.
     *
     *  This is the same as setting the style value to <code>undefined</code>.
     *
     *  @param styleProp The name of the style property.
     *  
     *  @langversion 3.0
     *  @playerversion Flash 9
     *  @playerversion AIR 1.1
     *  @productversion Flex 3
     */
    public function clearStyle(styleProp:String):void
    {
        public::setStyle(styleProp, undefined);
    }

    /**
     *  @private
     */
    mx_internal function createProtoChainRoot():Object
    {
        var root:Object = {};

        // If there's a defaultFactory for this style sheet,
        // then add the object it produces to the root.
        if (defaultFactory != null)
        {
            defaultFactory.prototype = root;
            root = new defaultFactory();
        }

        // If there's a factory for this style sheet,
        // then add the object it produces to the root.
        if (factory != null)
        {
            factory.prototype = root;
            root = new factory();
        }

        clones[ root ] = 1;
        
        return root;
    }

    /**
     *  @private
     */
    mx_internal function addStyleToProtoChain(chain:Object,
                                         target:DisplayObject,
                                         filterMap:Object = null):Object
    {
        var nodeAddedToChain:Boolean = false;
        var originalChain:Object = chain;
        
        if (filterMap)
        {
            chain = {};
        }
        
        // Add parent style to proto chain.
        if (styleManager.parent)
        {
            var parentStyle:CSSStyleDeclaration = styleManager.parent.getStyleDeclaration(selectorString);
            if (parentStyle)
                chain = parentStyle.addStyleToProtoChain(chain, target, filterMap);
        }
        
        // If there's a defaultFactory for this style sheet,
        // then add the object it produces to the chain.
        if (defaultFactory != null)
        {
            defaultFactory.prototype = chain;
            chain = new defaultFactory();
            nodeAddedToChain = true;
        }

        // If there's a factory for this style sheet,
        // then add the object it produces to the chain.
        var objectFactory:Object = null;
        if (factory != null)
        {
            objectFactory = new factory();
            factory.prototype = chain;
            chain = new factory();
            nodeAddedToChain = true;
        }
        
        // If someone has called setStyle() on this CSSStyleDeclaration,
        // then some of the values returned from the factory are
        // out-of-date. Overwrite them with the up-to-date values.
        if (overrides)
        {
            // Before we add our overrides to the object at the head of
            // the chain, make sure that we added an object at the head
            // of the chain.
            if (defaultFactory == null && factory == null)
            {
                var emptyObjectFactory:Function = function():void
                {
                };
                emptyObjectFactory.prototype = chain;
                chain = new emptyObjectFactory();
                nodeAddedToChain = true;
            }

            for (var p:String in overrides)
            {
                if (overrides[p] === undefined)
                    delete chain[p];
                else
                    chain[p] = overrides[p];
            }
        }

        if (filterMap)
        {
            if (nodeAddedToChain)
            {
            
                var filteredChain:Object = {};
                // Create an object on the head of the chain using the original chain       
                var filterObjectFactory:Function = function():void
                {
                };
                filterObjectFactory.prototype = originalChain;
                filteredChain = new filterObjectFactory();
                
                for (var i:String in chain)
                {
                    if (filterMap[i] != null)
                    {
                        filteredChain[filterMap[i]] = chain[i];
                    }
                } 
                
                chain = filteredChain;
                chain[FILTERMAP_PROP] = filterMap;
            }
            else
            {
                chain = originalChain;
            }
        }

        if (nodeAddedToChain)
            clones[chain] = 1;

        return chain;
    }

    /**
     *  @private
     */
    mx_internal function clearOverride(styleProp:String):void
    {
        if (overrides && overrides[styleProp] !== undefined)
            delete overrides[styleProp];
    }

    /**
     *  @private
     */
    private function clearStyleAttr(styleProp:String):void
    {
        // Put "undefined" into our overrides Array
        if (!overrides)
            overrides = {};
        overrides[styleProp] = undefined;
        
        // Remove the property from all our clones
        for (var clone:* in clones)
        {
            delete clone[styleProp];
        }
    }
    
    /**
     *  @private
     */
    mx_internal function updateClones(styleProp:String, value:*):void
    {
        // Update all clones of this style sheet.
        for (var clone:* in clones)
        {
            var cloneFilter:Object = clone[FILTERMAP_PROP];
            if (cloneFilter)
            {
                if (cloneFilter[styleProp] != null)
                {
                    clone[cloneFilter[styleProp]] = value;      
                }
            }
            else
            {
                clone[styleProp] = value;
            }
        }
    }
    
}

}
