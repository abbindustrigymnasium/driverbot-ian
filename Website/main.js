import * as THREE from 'three';
import { STLLoader } from 'three/examples/jsm/loaders/STLLoader';
import { TextureLoader } from 'three/src/loaders/TextureLoader';
import { GLTFLoader } from 'three/examples/jsm/loaders/GLTFLoader';

var scene = new THREE.Scene();

// Create a camera
var camera = new THREE.PerspectiveCamera(75, 1920 / 1080, 0.1, 1000);
camera.position.z = 5;

// Create a renderer
var renderer = new THREE.WebGLRenderer();
renderer.setSize(1920,1080);
renderer.setClearColor(0x000000, 0); // Set clear color to black with opacity 0 for transparency
renderer.domElement.style.overflow = 'hidden'; // Set overflow property to hidden
document.body.appendChild(renderer.domElement);

// Add ambient light to the scene
var ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
scene.add(ambientLight);
var stlMesh; // Variable to store the loaded STL mesh

// Load STL model
var loader = new STLLoader();
loader.load('output.stl', function (geometry) {
    var textureLoader = new THREE.TextureLoader();
    textureLoader.load('texture/Concrete/ConcretePrecastPlates004_DISP_2K_METALNESS.png', function (texture) {
        var material = new THREE.MeshPhongMaterial({ map: texture }); // Apply texture to the material
        var mesh = new THREE.Mesh(geometry, material);

        // Scale down the mesh
        mesh.scale.set(0.01, 0.01, 0.01);

        // Set the position coordinates
        mesh.position.set(-3.4, 2.4, 0);

        scene.add(mesh);

        // Assign the mesh to the stlMesh variable
        stlMesh = mesh;

        // Add directional light after the mesh is loaded
        var directionalLight = new THREE.DirectionalLight(0xffffff, 1);
        directionalLight.position.set(0, -2, 100); // Set the position of the light
        directionalLight.target = stlMesh; // Set the target of the light to the mesh
        scene.add(directionalLight);
    });
});

// Render loop
function animate() {
    requestAnimationFrame(animate);
    renderer.render(scene, camera);
}
animate();
