/* simple program to apply edge collapse a set number of times 
 */
#include <stdio.h> 
#include "mesh.h"
#include "pmesh.h"
#include <string>
#include <stdlib.h>
#include <algorithm>
#include "cmdline.h"

int cursurf=0;
bool insurf( triangle &t ) { return t.isActive() && t.getSurface()==cursurf; }


/** output reduced surface model
 *
 * \param name   file name
 * \param pmsh   reduced surface
 * \param msh    original surface
 */
void
reduced_mesh( string name, PMesh &pmsh, Mesh &msh  )
{
  // identify nodes in reduced model
  vector<bool> in_red_mod( msh.getNumVerts() );
  for( int i=0; i<pmsh.numTris(); i++ ) {
    triangle t;
    if ( pmsh.getTri(i, t) && t.isActive() ) {
      int a, b, c;
      t.getVerts( a, b, c );
      in_red_mod[a] = true;      
      in_red_mod[b] = true;      
      in_red_mod[c] = true;
    }
  }

  // determine mapping of old to new
  vector<int> pt_map( msh.getNumVerts(), -1 );
  int numpt=0;
  for( int i=0; i<msh.getNumVerts(); i++ ) {
    if( in_red_mod[i] )
      pt_map[i] = numpt++;
  }

  string pfile = name+".pts";
  FILE *fout = fopen( pfile.c_str(), "w" );
  fprintf( fout, "%d\n", numpt );
  for( int i=0; i<msh.getNumVerts(); i++ )
    if( in_red_mod[i] ) {
      const  float *vc = msh.getVertex(i).getArrayVerts();
      fprintf( fout, "%f %f %f\n", vc[0], vc[1], vc[2] );
    }
  fclose( fout );

  string efile = name+".surf";
  fout = fopen( efile.c_str(), "w" );
  int s=0;
  list<string> surfnm =msh.surface_names();
  for(list<string>::iterator it=surfnm.begin(); it!=surfnm.end(); it++) {
    cursurf = s++;
    int numtri = count_if( pmsh.triangle_list().begin(), pmsh.triangle_list().end(), insurf );
    fprintf( fout, "%d %s_red\n", numtri, it->c_str() );
    for (int i=0; i<pmsh.numTris(); i++) {
      triangle t;
      if ( pmsh.getTri(i, t) && t.isActive() && t.getSurface()==cursurf ) {
        int a, b, c;
        t.getVerts( a, b, c );
        fprintf( fout, "Tr %d %d %d 1\n", pt_map[a], pt_map[b], pt_map[c] );
      }
    }
  }
  fclose( fout );
}


/** output reduced surface mesh
 *
 * \param name   file name
 * \param pmsh   reduced surface
 * \param msh    original surface
 * \param pr_pts print points if true
 */
void
dump_mesh( string name, PMesh &pmsh, Mesh &msh, bool pr_pts  )
{
  string efile = name+".surf";
  FILE *fout = fopen( efile.c_str(), "w" );
  list<string> surfnm =msh.surface_names();

  int s=0;
  for(list<string>::iterator it=surfnm.begin(); it!=surfnm.end(); it++) {
    cursurf = s++;
    int numtri = count_if( pmsh.triangle_list().begin(), pmsh.triangle_list().end(), insurf );
    fprintf( fout, "%d %s_red\n", numtri, it->c_str() );
    for (int i=0; i<pmsh.numTris(); i++) {
      triangle t;
      if ( pmsh.getTri(i, t) && t.isActive() && t.getSurface()==cursurf ) {
        int a, b, c;
        t.getVerts( a, b, c );
        fprintf( fout, "Tr %d %d %d 1\n", a, b, c );
      }
    }
  }
  fclose( fout );

  if( pr_pts ) {
    string pfile = name+".pts";
    fout = fopen( pfile.c_str(), "w" );
    fprintf( fout, "%d\n", msh.getNumVerts() );
    for( int i=0; i<msh.getNumVerts(); i++ ){
      vertex v = msh.getVertex(i);
      const  float *vc = v.getArrayVerts();
      fprintf( fout, "%f %f %f\n", vc[0], vc[1], vc[2] );
    }
    fclose( fout );
  }
  
}


int
main( int argc, char *argv[] )
{
  gengetopt_args_info args;
  if( argc<2 ) {
    cmdline_parser_print_help();
    exit(0);
  }

  // let's call our cmdline parser 
  if (cmdline_parser (argc, argv, &args) != 0)
    exit(1);
  
  Mesh fine_mesh( args.inputs[0], true );
  
  PMesh::EdgeCost ecost;
  switch( args.method_arg ) {
      case  method_arg_shortest:
          ecost = PMesh::SHORTEST;
          break;
      case  method_arg_melax:
          ecost = PMesh::MELAX;
          break;
      case  method_arg_quadric:
          ecost = PMesh::QUADRIC;
          break;
      case  method_arg_quadrictri:
          ecost = PMesh::QUADRICTRI;
          break;
      case  method_arg_maxedgecost:
          ecost = PMesh::MAX_EDGECOST;
          break;
  }

  PMesh  pmesh( &fine_mesh, ecost );

  int numTri = fine_mesh.getNumTriangles();
  if( args.percent_given ) {
    numTri *= args.percent_arg;
    numTri /= 100;
  } else if( args.numtri_given ) 
    numTri = args.numtri_arg;
  else {
    fprintf( stderr, "Need to specify how many triangles to retain\n" );
    exit(1);
  }
  
  while( pmesh.numVisTris() > numTri )
    pmesh.collapseEdge();

  cout << "mesh now has " << pmesh.numVisTris() << " triangles" << endl;

  dump_mesh( args.name_arg, pmesh, fine_mesh, args.pts_flag );

  if( args.redmod_given )
    reduced_mesh( args.redmod_arg, pmesh, fine_mesh );
}

