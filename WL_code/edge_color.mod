############################################################
#Indices
############################################################
param E > 0 integer;
param C > 0 integer;

############################################################
#Constants and Sets
############################################################
set colors := 0..C-1;
set links := 0..E-1;

param s {e in links, ee in links} binary;

############################################################
#Variables
############################################################
var x {i in colors, e in links} binary;

############################################################
#Objective Function
############################################################
maximize ColoredEdge: sum {e in links} (sum {i in colors} x[i, e]);

############################################################
#Constraints
############################################################
subject to edge_color_sum {e in links}: sum {i in colors} (x[i, e]) <= 1;
subject to edge_share {i in colors, e in links, ee in links: e != ee}: s[e, ee] * (x[i, e] + x[i, ee]) <= 1;

