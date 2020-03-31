$fn=100;

single();
mirror([1,0,0])single();

//color("green")translate([0,22,3])cube([18,44,1],center=true);
//color("silver")translate([0,(7.5/2)-1.5,1])cube([9,7.5,3],center=true);

module single(){
    difference(){
        union(){
            translate([10.75,2.5,-0.75])cube([6.5,5,6.5],center=true);
            translate([0,2.5,5])cube([28,5,5],center=true);
            translate([0,24.25,5.5])cube([10,44.5,4],center=true);
            translate([0,45.25,2])cube([10,2.5,4],center=true);
        }union(){
            translate([9.6,-1,0])rotate([-90,0,0])cylinder(d=2.5,h=10);
            translate([0,21,3])cube([18.8,44.8,1],center=true);
        }
    }
}