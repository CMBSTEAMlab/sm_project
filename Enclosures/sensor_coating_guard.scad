inner_width = 40;
top_height = 14;
bottom_height = 10;
inner_depth = 16;
inner_point_offset = 3;
inner_point_diameter = 2;

connector_gap = 8;

point_offset = 7.5;
point_diameter =  2;
width = 47.5;
depth = 27.5;

gap = 1;

wiper_width = 14;
wiper_depth = 16;
wiper_thickness = 5;
wiper_inset = 4;
wiper_gap = 3;

$fs = .125;
$fa = .125;

top_guard();
//wiper();

module wiper() {
    difference() {
        cube([wiper_width, wiper_depth, wiper_thickness]);
        hull() {
            translate([wiper_width/2-wiper_gap/2, -1, -1]) cube([wiper_gap, wiper_inset+1, wiper_thickness+2]);
            translate([wiper_width/2, wiper_inset, -1]) cylinder(r=wiper_gap/2, h=wiper_thickness+2);
        }
    }
}


module top_guard() {
    difference() {
        union() {
            hull() {
                cube([inner_width,inner_depth,top_height]);
                 translate([inner_width+inner_point_offset, inner_depth/2, 0]) cylinder(r=inner_point_diameter/2,h=top_height);   
            }
            hull() {
                translate([-(width-inner_width), -(depth-inner_depth)/2, -gap]) cube([width, depth, top_height]);
                 translate([inner_width+point_offset, inner_depth/2, -gap]) cylinder(r=inner_point_diameter/2,h=top_height);   
            }
        }
        hull() {
            translate([-(width-inner_width)-1, (inner_depth-connector_gap)/2, -gap-1]) cube([width-inner_width-connector_gap/2+1, connector_gap, top_height+2]);
            translate([-connector_gap/2, inner_depth/2, -gap]) cylinder(r=connector_gap/2,h=top_height+2);   

        }
    }
}

module bottom_guard() {
    difference() {
        union() {
            hull() {
                cube([inner_width,inner_depth,top_height]);
                 translate([inner_width+inner_point_offset, inner_depth/2, 0]) cylinder(r=inner_point_diameter/2,h=top_height);   
            }
            hull() {
                translate([-(width-inner_width), -(depth-inner_depth)/2, -gap]) cube([width, depth, top_height]);
                 translate([inner_width+point_offset, inner_depth/2, -gap]) cylinder(r=inner_point_diameter/2,h=top_height);   
            }
        }
        hull() {
            translate([-(width-inner_width)-1, (inner_depth-connector_gap)/2, -gap-1]) cube([width-inner_width-connector_gap/2+1, connector_gap, top_height+2]);
            translate([-connector_gap/2, inner_depth/2, -gap]) cylinder(r=connector_gap/2,h=top_height+2);   

        }
    }
}