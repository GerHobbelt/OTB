/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef otb_GlView_h
#define otb_GlView_h


#include "otbGeoInterface.h"
#include "otbGlActor.h"
#include "otbViewSettings.h"

#include <map>
#include <vector>
#include <cassert>

namespace otb
{

/** 
 * The GlView class acts like an OpenGl scene where actors deriving
 * from the GlActor class can be rendered. The GlView class contains the
 * OpenGl viewport and manages:
 * - The size of the viewport,
 * - The entire refresh loop, including light or heavy rendering of
 * all actors and all OpenGl specific stuff needed before and after
 * the actors update,
 * - The actors stack (order in which actors are rendered).
 * 
 * All parameters related to scene description (origin, spacing, angle
 * ...) are stored and managed by the ViewSettings class.
 */
class GlView 
  : public itk::Object
{
public:
  typedef GlView                                          Self;
  typedef itk::Object                                     Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  typedef otb::GlActor                                    ActorType;
  typedef std::map<std::string,ActorType::Pointer>        ActorMapType;
  typedef std::vector<std::string>                        StringVectorType;

  /**
   * Type definition for otb::GlActor storage key.
   */
  typedef StringVectorType::value_type KeyType;

  itkNewMacro(Self);

  /**
   * The Intialize method will reset the OpenGl viewport to the given
   * size, clear view settings and remove any existing actor.
   * \param sx Width of the viewport
   * \param sy Height of the viewport
   */ 
  void Initialize(unsigned int sx, unsigned int sy);

  /**
   * This method allows to add a new actor (deriving from GlActor) to
   * the GlView. The actor can be identified by an optional key. If
   * not provided, and the default value is used, the method will
   * generate a key to identify the actor. In both case, the key is
   * returned by the method.  
   * \param actor The actor to be added
   * \param key The key to be used to identify the actor (default to
   * empty string)
   * \return The key identifying the actor (either passed to the
   * method or generated by it).
   */
  std::string AddActor(ActorType * actor, const std::string & key = "");

  /** 
   * This method will try to remove the actor identified by the given
   * key.
   * \param key The key identifying the actor to remove
   * \return True if the actor has been found and removed, false
   * otherwise
   */
  bool RemoveActor(const std::string & key);

  /**
   * This method will remove all existing actors at once.
   */
  void ClearActors();

  /**
   * This method allows to retrieve a pointer to the actor identified
   * by the given key.
   * \param key The key identifying the actor to retrieve
   * \return A pointer to the retrieved actor. This pointer will be
   * null if no actor could be found with this key.
   */  
  ActorType::Pointer GetActor(const std::string & key) const;

  /**
   * Tells wether an otb::GlActor is contained given its storage key.
   *
   * @param key otb::GlActor storage key.
   *
   * @return true if an otb::GlActor is contained given storage key.
   */
  bool ContainsActor( const KeyType & key ) const;

  /**
   * This method will return a vector containing the keys of all
   * actors.
   * \return A vector of string containing the keys of all actors.
   */ 
  std::vector<std::string> GetActorsKeys() const;

  /**
   * This method handles all the things that should be done before
   * rendering.
   */
  void BeforeRendering();

  /**
   * This method handles all the things that should be after before
   * rendering.
   */
  void AfterRendering();

  /**
   * This method will update the rendering of the OpenGl viewport,
   * taking into account all parameters in the ViewSettings, without
   * fetching any missing data from disk or RAM. It is therefore very
   * fast.
   */
  void LightRender();

  /**
   * This method will update the rendering of the OpenGl viewport,
   * taking into account all parameters in the ViewSettings, but will
   * first compute and load any missing data for a complete
   * rendering. As such, this update routine can be time consuming.
   */
  void HeavyRender();

  // Resize viewport
  void Resize(unsigned int sx, unsigned int sy);

  itkSetObjectMacro(Settings,ViewSettings);
  itkGetObjectMacro(Settings,ViewSettings);
  itkGetConstObjectMacro(Settings,ViewSettings);

  //comment this macro (not compiling with OTB 3.X)
  // Get Rendering order
  const StringVectorType & GetRenderingOrder() const
  {
    return m_RenderingOrder;
  };

  /**
   * Arbitrary set the rendering order of some or all of contained
   * otb::GlActor instances.
   *
   * Keys which are not contained will be ignored.
   *
   * @param keys The ordered sequence of keys.
   * @param front <code>true</code> to order selected otb::GlActor
   *              instances in front of non-selected ones.
   */
  void SetRenderingOrder( const StringVectorType & keys,
                          bool front );

  // This will rotate the rendering order (without modifying the order)
  void RotateRenderingOrder(bool down = false);

  // Move actor in rendering order
  void MoveActorInRenderingOrder(std::string key, bool down = false);

  // Move actor to the end of rendering order (either front if front
  // is set to true or back if front is set to false)
  void MoveActorToEndOfRenderingOrder(std::string key, bool front = false);

  /**
   * Reproject point and spacing expressed in viewport coordinate
   * system into given actor coordinate system.
   */
  template< typename P, typename S, typename P2, typename S2 >
  bool ReprojectFromView( P & center,
			  S & spacing,
			  const KeyType & key,
			  const P2 & vcenter,
			  const S2 & vspacing,
			  double norm = 1000.0 ) const;

  /**
   * Reproject viewport center and spacing into given actor's
   * coordinate system.
   */
  // bool Reproject( const KeyType & key, double  norm );

  /**
   */
  template< typename P >
  void GetExtent( P & origin, P & extent ) const;

protected:
  GlView();

  virtual ~GlView();

private:
  // prevent implementation
  GlView(const Self&);
  void operator=(const Self&);

  ViewSettings::Pointer m_Settings;

  ActorMapType          m_Actors;

  StringVectorType      m_RenderingOrder;

}; // End class GlView


template< typename P, typename S, typename P2, typename S2 >
bool
GlView
::ReprojectFromView( P & center,
		     S & spacing,
		     const KeyType & key,
		     const P2 & vcenter,
		     const S2 & vspacing,
		     double norm ) const
{
  //
  // Reference actor has not been found.
  otb::GlActor::Pointer actor( GetActor( key ) );

  if( actor.IsNull() )
    return false;


  //
  // Reference actor does not implement geo-interface.
  const otb::GeoInterface * geo =
    dynamic_cast< const GeoInterface * >( actor.GetPointer() );

  if( geo==NULL )
    return false;


  //
  // Compute transform origin.
  if( !geo->TransformFromViewport( center, vcenter, true ) )
    return false;

  //
  // Compute transformed X-axis extremity.
  GeoInterface::Point2d x( vcenter );

  x[ 0 ] += norm * vspacing[ 0 ]; 

  if( !geo->TransformFromViewport( x, x, true ) )
    return false;

  //
  // Compute transformed Y-axis extremity.
  GeoInterface::Point2d y( vcenter );

  y[ 1 ] += norm * vspacing[ 1 ];

  if( !geo->TransformFromViewport( y, y, true ) )
    return false; 

  //
  // Compute transformed spacing.
  //
  // Note SAT:
  //     Formula has been taken from IceViewer::key_callback(). I think
  // that the norm of the transformed X and Y axises is not
  // the new spacing if transform contains a rotation.
  //     To correct this, transformed X and Y vectors should be
  // projected against reference actor X and Y axises (using vectorial
  // dot product).

  x[ 0 ] -= center[ 0 ];
  x[ 1 ] -= center[ 1 ];

  y[ 0 ] -= center[ 0 ];
  y[ 1 ] -= center[ 1 ];

  spacing[ 0 ] = vcl_sqrt( x[ 0 ] * x[ 0 ] + x[ 1 ] * x[ 1 ] ) / norm;
  spacing[ 1 ] = vcl_sqrt( y[ 0 ] * y[ 0 ] + y[ 1 ] * y[ 1 ] ) / norm;

  // Sign of x-spacing is done by sign( x . (1, 0) ) which is sign( x[ 0 ] )
  // Sign of y-spacing is done by sign( y . (0, 1) ) which is sign[ y[ 1 ] )

  if( x[ 0 ]<0.0 )
    spacing[ 0 ] = -spacing[ 0 ];

  if( y[ 1 ]<0.0 )
    spacing[ 1 ] = -spacing[ 1 ];

  return true;
}


template< typename P >
void
GlView
::GetExtent( P & origin, P & extent ) const
{
  if( m_Actors.empty() )
    {
    origin[ 0 ] = origin[ 1 ] = 0;
    extent[ 0 ] = extent[ 1 ] = 0;

    return;
    }


  origin[ 0 ] = std::numeric_limits< typename P::ValueType >::infinity();
  origin[ 1 ] = std::numeric_limits< typename P::ValueType >::infinity();

  extent[ 0 ] = -std::numeric_limits< typename P::ValueType >::infinity();
  extent[ 1 ] = -std::numeric_limits< typename P::ValueType >::infinity();

  for( ActorMapType::const_iterator it( m_Actors.begin() );
       it!=m_Actors.end();
       ++it )
    {
    P o;
    P e;

    assert( !it->second.IsNull() );

    it->second->GetExtent( o[ 0 ], o[ 1 ], e[ 0 ], e[ 1 ] );


    if( o[ 0 ]<origin[ 0 ] )
      origin[ 0 ] = o[ 0 ];

    if( o[ 1 ]<origin[ 1 ] )
      origin[ 1 ] = o[ 1 ];

    if( o[ 0 ]>extent[ 0 ] )
      extent[ 0 ] = o[ 0 ];

    if( o[ 1 ]>extent[ 1 ] )
      extent[ 1 ] = o[ 1 ];


    if( e[ 0 ]<origin[ 0 ] )
      origin[ 0 ] = e[ 0 ];

    if( e[ 1 ]<origin[ 1 ] )
      origin[ 1 ] = e[ 1 ];

    if( e[ 0 ]>extent[ 0 ] )
      extent[ 0 ] = e[ 0 ];

    if( e[ 1 ]>extent[ 1 ] )
      extent[ 1 ] = e[ 1 ];
    }
}


} // End namespace otb

#endif
