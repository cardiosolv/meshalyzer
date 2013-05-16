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


main( int argc, char *argv[] )
{
  gengetopt_args_info args;

  // let's call our cmdline parser 
  if (cmdline_parser (argc, argv, &args) != 0)
    exit(1);
  
  Mesh   fine_mesh( args.inputs[0], true );
  PMesh  pmesh( &fine_mesh, PMesh::QUADRICTRI );

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
}

