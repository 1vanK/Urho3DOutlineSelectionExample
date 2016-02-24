# Urho3D Outline Selection Example

Tuning: http://urho3d.prophpbb.com/topic1840.html <br>
See also: https://github.com/1vanK/Urho3DOutline

License: Public Domain

If you do not want to see outline through other objects then remove <b>depthtest="always"</b> in technique:
```
- <pass name="outline" vs="Outline" ps="Outline" psdefines="MASK" depthtest="always" depthwrite="false" />
+ <pass name="outline" vs="Outline" ps="Outline" psdefines="MASK" depthwrite="false" />
```

Controls: Press RMB for moving and rotating camera, and LMB for select

![screenshot](https://raw.githubusercontent.com/1vanK/Urho3DOutlineSelectionExample/master/Screen.jpg)


