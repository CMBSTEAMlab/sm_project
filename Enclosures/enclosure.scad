inches2mm = 25.4;

battery_height = 2.5*inches2mm;

battery_holder_width = 2.25*inches2mm;
battery_holder_depth = 1.5*inches2mm;
battery_holder_height = 1*inches2mm;
battery_fit_tol = 1;

battery_slider_extra_width = 5;
battery_slider_extra_depth = 5;
battery_slider_extra_height = 5;
battery_slider_gasket_thickness = 1.5;

battery_lip_width = 5;
battery_lip_pitch = 5;

battery_offset = 7;

battery_slider_pitch = 10;
battery_slider_width = 5;

mount_hole_diameter = 16.5;
mount_attachment_width = 1.5*inches2mm;
mount_attachment_height = 2.5*inches2mm;
mount_attachment_depth = 1/8*inches2mm;

bolt_diameter = 4.7;
hex_diameter = 8; // M4 nut
hex_thickness = 3; // M4 nut
//hex_diameter = 9.7; // #8 nut
//hex_thickness = 3.2; // #8 nut

post_diameter = 3/8*inches2mm;
post_hole_diameter = 3;
post_height = 3/8*inches2mm;
post_separation = 2*inches2mm;
post_offset = 1/2*inches2mm;

l_hole_diameter = post_hole_diameter+.7;
l_width = 2.5*inches2mm;
l_depth = 2*inches2mm;
l_height = 3.5*inches2mm;
l_offset = 4;
l_thickness = 5;

cover_height = 4*inches2mm;

holder_width = 3*inches2mm;
holder_depth = 2.5*inches2mm;
holder_height = 2*inches2mm;
thickness = 3;
base_thickness = 6;
lip_width = 3/8*inches2mm;
//lip_width = hex_diameter+1;
lip_pitch = 30;

wire_hole_diameter = 1/4*inches2mm;
power_hole_diameter = 1/4*inches2mm;

fit_tol = 1; // space between cover and holder when holder is inserted, adjust to get desired fit

separation = 10; // space between cover and holder, just for display purposes
battery_separation = 5;
battery_holder_separation = 1.5;
l_separation = 3;

square_off_width = lip_width-.1;
square_off_height = base_thickness;
square_off_z = lip_pitch;
$fs = .25;
$fa = .25;

//cover();
//holder();
//l_mount();
//battery_base();
battery_holder();
//battery_slider();

cover_width = holder_width+2*fit_tol+2*thickness;
cover_depth = holder_depth+2*fit_tol+2*thickness;

base_width = cover_width+2*thickness+2*lip_width;
base_depth = cover_depth+2*thickness+2*lip_width;

full_cover_height = cover_height+thickness;

bolt_height =  2*thickness+separation+battery_separation+2+lip_pitch;
bolt_z = -separation-2*thickness-battery_separation-1;


battery_width = battery_holder_width+2*thickness+2*battery_fit_tol;
battery_depth = battery_holder_depth+2*thickness+2*battery_fit_tol;    
battery_base_height = thickness+battery_height;

module cover() {
  difference() {
    union() {
      hull() {
        cube([base_width, base_depth, base_thickness]);
        translate([lip_width, lip_width, lip_pitch-thickness]) cube([base_width-2*lip_width, base_depth-2*lip_width, thickness]);

      }
      hull() {
          translate([base_width/2, base_depth-lip_width+mount_attachment_depth/2-1, cover_height-mount_attachment_height/2+thickness]) cube([mount_attachment_width, mount_attachment_depth, mount_attachment_height], center=true);
          translate([base_width/2, base_depth+10+mount_attachment_depth/2-1, cover_height-mount_attachment_height/2+thickness]) cylinder(r=mount_hole_diameter/2+thickness, h=mount_attachment_height, center=true);
          
      }
      translate([lip_width, lip_width, 0]) 
        cube([cover_width+2*thickness, cover_depth+2*thickness, cover_height+thickness]);
    }
    
    translate([lip_width+thickness,lip_width+thickness,-1]) 
      cube([cover_width, cover_depth, cover_height+1]); 
    hull() {
      translate([.75*lip_width+thickness,.75*lip_width+thickness,-thickness]) 
        cube([cover_width+.5*lip_width, cover_depth+.5*lip_width, thickness]); 
      translate([lip_width+thickness,lip_width+thickness,lip_pitch-thickness]) 
        cube([cover_width, cover_depth, thickness]);
    }
    bolts_and_hexes();
    wire_hole();
    power_hole();
    translate([base_width/2, base_depth+10+mount_attachment_depth/2-1, cover_height-mount_attachment_height/2]) cylinder(r=mount_hole_diameter/2, h=mount_attachment_height, center=true);
  }
}

module holder() {
difference() {
union() {
  difference() {
    union() {
        translate([0,0,-separation-thickness]) cube([base_width, base_depth, thickness]);
        translate([lip_width+thickness+fit_tol,lip_width+thickness+fit_tol,-separation-1]) 
          cube([cover_width-2*fit_tol, cover_depth-2*fit_tol, holder_height]);
    }
    translate([lip_width+thickness+fit_tol+thickness,lip_width+thickness+fit_tol+thickness,-separation]) 
          cube([cover_width-2*fit_tol-2*thickness, cover_depth-2*fit_tol-2*thickness, holder_height+1]);
    bolts_and_hexes();
    wire_hole();
    power_hole();
  }
  translate([base_width/2-post_separation/2,base_depth/2+post_offset,-separation-1]) cylinder(r=post_diameter/2, h=post_height+1);
    translate([base_width/2+post_separation/2,base_depth/2+post_offset,-separation-1]) cylinder(r=post_diameter/2, h=post_height+1);

}
  l_holes(post_hole_diameter);
}
}

module l_holes(d) {
      translate([base_width/2-post_separation/2,base_depth/2+post_offset,-separation]) cylinder(r=d/2, h=post_height+1+l_separation+l_thickness);
    translate([base_width/2+post_separation/2,base_depth/2+post_offset,-separation]) cylinder(r=d/2, h=post_height+1+l_separation+l_thickness);
}

module l_mount() {
    
    difference() {
      union() {
        translate([base_width/2, base_depth/2-l_offset, -separation+post_height+l_thickness/2+l_separation]) cube([l_width, l_depth, l_thickness], center=true); 
        //translate([
      }
      l_holes(l_hole_diameter);
    }
    
}

module bolts_and_hexes() {
    //bolts
    translate([lip_width/2,lip_width/2,bolt_z]) cylinder(r=bolt_diameter/2, h =bolt_height);
    translate([base_width-lip_width/2,lip_width/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    translate([base_width-lip_width/2,base_depth-lip_width/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    translate([lip_width/2,base_depth-lip_width/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    translate([base_width/2,lip_width/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    translate([base_width/2,base_depth-lip_width/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    translate([base_width-lip_width/2,base_depth/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    translate([lip_width/2,base_depth/2,bolt_z]) cylinder(r=bolt_diameter/2, h = bolt_height);
    
    // square off bolt holes on cover slant
    translate([lip_width/2,lip_width/2,square_off_height+square_off_z/2]) cube([square_off_width, square_off_width,square_off_z], center=true);
    translate([base_width-lip_width/2,lip_width/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    translate([base_width-lip_width/2,base_depth-lip_width/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    translate([lip_width/2,base_depth-lip_width/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    translate([base_width/2,lip_width/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    translate([base_width/2,base_depth-lip_width/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    translate([base_width-lip_width/2,base_depth/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    translate([lip_width/2,base_depth/2,square_off_height+square_off_z/2]) cube([square_off_width,square_off_width,square_off_z], center=true);
    
    // slots for inserted hex nuts
    translate([lip_width/2,lip_width/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
    translate([base_width-lip_width/2,lip_width/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
    translate([base_width-lip_width/2,base_depth-lip_width/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
    translate([lip_width/2,base_depth-lip_width/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
    translate([base_width/2,base_depth-lip_width/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
     translate([base_width/2,lip_width/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
     translate([lip_width/2,base_depth/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
    translate([base_width-lip_width/2,base_depth/2,base_thickness-hex_thickness]) cylinder(r=hex_diameter/2, h = hex_thickness+1, $fn=6);
}

module wire_hole() {
    translate([base_width/2,base_depth-(lip_width+thickness+wire_hole_diameter+fit_tol), -battery_separation-separation-2*thickness-1]) cylinder(r=wire_hole_diameter/2, h=2*thickness+separation+battery_separation+2);
}
module power_hole() {
    translate([base_width/2,base_depth/2, -battery_separation-separation-2*thickness-1]) cylinder(r=power_hole_diameter/2, h=2*thickness+separation+battery_separation+2);
}

module battery_base() {
    difference() {
      translate([0,0,-separation-battery_separation-thickness]) local_battery_base();
      bolts_and_hexes();
      wire_hole();
      power_hole();
    }
}

module local_battery_base() {
  difference() {
    union() {
      translate([base_width/2,base_depth/2-battery_offset,-thickness-battery_height/2]) cube([battery_width+2*thickness, battery_depth+2*thickness, battery_height], center=true);
        
      translate([0,0,-thickness]) cube([base_width, base_depth, thickness]);
      translate([0,-battery_offset, 0]) hull() {
          translate([base_width/2, base_depth/2, -thickness-battery_height-.5+battery_lip_pitch]) cube([battery_width+2*thickness, battery_depth+2*thickness, 1], center=true);
          translate([base_width/2, base_depth/2, -thickness-battery_height+.5]) cube([battery_width+2*battery_lip_width+2*thickness, battery_depth+2*battery_lip_width+2*thickness, 1], center=true);
      }
    }
    
    translate([base_width/2,base_depth/2-battery_offset,-thickness-battery_height/2-1]) cube([battery_width, battery_depth, battery_height+2], center=true);
   translate([0,-battery_offset, 0]) hull() {
          translate([base_width/2, base_depth/2, -thickness-battery_height-.5+battery_holder_height/2]) cube([battery_width, battery_depth, 1], center=true);
          translate([base_width/2, base_depth/2, -thickness-battery_height-.5]) cube([battery_width+battery_fit_tol+1, battery_depth+battery_fit_tol+1, 1], center=true);
      }
  }
}

module battery_holder() {
  translate([0,0,-separation-thickness-battery_separation-battery_base_height-battery_holder_separation]) local_battery_holder();
}

module local_battery_holder() {
    difference() {
    union() {
  translate([0,-battery_offset, 0]) hull() {
    translate([base_width/2, base_depth/2, -.5]) cube([battery_width+2*battery_lip_width+2*thickness, battery_depth+2*battery_lip_width+2*thickness, 1], center=true);
    
    translate([base_width/2, base_depth/2, .5-battery_lip_pitch]) cube([battery_width+2*thickness, battery_depth+2*thickness, 1], center=true);
    
  }
  translate([base_width/2, base_depth/2-battery_offset, battery_holder_height/2]) cube([battery_holder_width+2*thickness, battery_holder_depth+2*thickness, battery_holder_height], center=true);
  
  }
  
  }
}

module battery_slider() {
    translate([base_width/2,base_depth/2-battery_offset,-separation-thickness-battery_separation-battery_base_height-battery_holder_separation/2]) local_battery_slider();
}

slider_width = battery_width+2*thickness+2*battery_lip_width+battery_slider_extra_width;
slider_depth = battery_depth+2*thickness+2*battery_lip_width+battery_slider_extra_height;
slider_height = 2*battery_lip_pitch+battery_slider_gasket_thickness+battery_slider_extra_height;
module local_battery_slider() {
    difference() {
      cube([slider_width,slider_depth,slider_height],center = true);
      
      hull() {
        translate([0, 0, -.5+battery_slider_gasket_thickness/2+battery_lip_pitch]) cube([battery_width+2*thickness, battery_depth+2*thickness, 1], center=true);
        translate([0, 0, .5+battery_slider_gasket_thickness/2]) cube([battery_width+2*thickness+2*battery_lip_width, battery_depth+2*thickness+2*battery_lip_width, 1], center=true);
        translate([0, 0, -(.5+battery_slider_gasket_thickness/2)]) cube([battery_width+2*thickness+2*battery_lip_width, battery_depth+2*thickness+2*battery_lip_width, 1], center=true);
        translate([0, 0, -(-.5+battery_slider_gasket_thickness/2+battery_lip_pitch)]) cube([battery_width+2*thickness, battery_depth+2*thickness, 1], center=true);
      }
      cube([battery_width+2*thickness, battery_depth+2*thickness, slider_height+2], center=true);
      translate([0,battery_depth/2+thickness+(battery_lip_width+battery_slider_extra_depth/2)/2,0]) cube([battery_width+2*thickness, battery_lip_width+battery_slider_extra_depth/2+2, slider_height+2], center=true);

      
    }
}
