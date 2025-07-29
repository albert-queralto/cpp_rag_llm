import React, { useState, useEffect } from "react";
import axios from "axios";
import ReactModal from "react-modal";
import { Tooltip } from 'react-tooltip';

ReactModal.setAppElement("#root");

const CollectionsManagement = () => {
    const [collections, setCollections] = useState([]);
    const [collectionName, setCollectionName] = useState("");
    const [newCollectionName, setNewCollectionName] = useState("");
    const [isModalOpen, setIsModalOpen] = useState(false);
    const [version, setVersion] = useState("");
    const [heartbeat, setHeartbeat] = useState("");
    const baseUrl = "http://localhost:4000/api/collections";
    const [currentCollection, setCurrentCollection] = useState("");

    useEffect(() => {
        fetchCollections();
        fetchVersion();
        fetchHeartbeat();
    }, []);

    const fetchCollections = async () => {
        try {
            const response = await axios.get(baseUrl);
            setCollections(response.data);
        } catch (error) {
            console.error("Error fetching collections:", error);
        }
    };

    const fetchVersion = async () => {
        try {
            const response = await axios.get("http://localhost:4000/api/chroma/version");
            setVersion(response.data);
        } catch (error) {
            console.error("Error fetching version:", error);
        }
    };

    const fetchHeartbeat = async () => {
        try {
            const response = await axios.get("http://localhost:4000/api/chroma/heartbeat");
            setHeartbeat(response.data);
        } catch (error) {
            console.error("Error fetching heartbeat:", error);
        }
    };

    const createCollection = async () => {
        if (!collectionName.trim()) {
            console.error("Collection name is required");
            return;
        }

        try {
            await axios.post(`${baseUrl}/${encodeURIComponent(collectionName)}`);
            fetchCollections();
            setCollectionName("");
        } catch (error) {
            console.error("Error creating collection:", error);
        }
    };

    const updateCollection = async (collectionNameToUpdate) => {
        try {
            await axios.put(`${baseUrl}/${encodeURIComponent(collectionNameToUpdate)}`, {
                name: newCollectionName
            });
            fetchCollections();
            closeModal();
        } catch (error) {
            console.error("Error updating collection:", error);
        }
    };

    const deleteCollection = async (collectionName) => {
        try {
            await axios.delete(`${baseUrl}/${encodeURIComponent(collectionName)}`);
            fetchCollections();
        } catch (error) {
            console.error("Error deleting collection:", error);
        }
    };

    const openModal = (collectionName) => {
        setCurrentCollection(collectionName);
        setIsModalOpen(true);
    };

    const closeModal = () => {
        setIsModalOpen(false);
        setCurrentCollection("");
        setNewCollectionName("");
    };

    const handleUpdate = (e) => {
        e.preventDefault();
        updateCollection(currentCollection);
    };

    return (
        <div>
            <header className="chat-header">
                <div style={{ flexGrow: 1, display: "flex", justifyContent: "center" }}>
                    Manage Collections
                </div>
            </header>
            <div className="container d-flex justify-content-center">
                <div className="row d-flex justify-content-center" style={{ marginTop: '10px', marginBottom: '10px' }}>
                    <div className="col-8">
                        <input
                            type="text"
                            placeholder="Name"
                            className="rounded-pill"
                            style={{ fontSize: '1.25rem' }}
                            value={collectionName}
                            onChange={(e) => setCollectionName(e.target.value)}
                        />
                    </div>
                    <div className="col-4">
                        <button
                            id="add-tooltip"
                            type="button"
                            className="rounded-pill"
                            style={{ fontSize: '1.25rem' }}
                            onClick={createCollection}><i className="bi bi-plus-lg"></i>
                        </button>
                        <Tooltip anchorSelect="#add-tooltip" content="Add Collection" />
                    </div>
                </div>
            </div>
            <div style={{ display: 'flex', justifyContent: 'center' }}>
                <table style={{ borderCollapse: 'separate', borderSpacing: '20px 20px', textAlign: 'center', fontSize: '1.25rem' }}>
                    <thead>
                        <tr>
                            <th>Name</th>
                            <th>Actions</th>
                        </tr>
                    </thead>
                    <tbody>
                        {collections.map((collection, index) => (
                            <tr key={index}>
                                <td>{collection.name}</td>
                                <td>
                                    <button
                                        id="edit-tooltip"
                                        type="button"
                                        className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
                                        style={{ fontSize: '1.25rem' }}
                                        onClick={() => openModal(collection.name)}><i className="bi bi-pencil-square"></i>
                                    </button>
                                    <Tooltip anchorSelect="#edit-tooltip" content="Edit Collection" />
                                    <button
                                        id="delete-tooltip"
                                        type="button"
                                        className="block text-lg py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill"
                                        style={{ fontSize: '1.25rem' }}
                                        onClick={() => deleteCollection(collection.name)}><i className="bi bi-x-lg"></i>
                                    </button>
                                    <Tooltip anchorSelect="#delete-tooltip" content="Delete Collection" />
                                </td>
                            </tr>
                        ))}
                    </tbody>
                </table>
            </div>
            <ReactModal 
                isOpen={isModalOpen}
                onRequestClose={closeModal}
                contentLabel="Update Collection Name"
                style={{ content: { backgroundColor: '#0e2f48' } }}
            >
                <h2 style={{ color: 'white' }}>Update Collection Name</h2>
                <div className="modal-content">
                    <span className="close" onClick={closeModal}>&times;</span>
                    <form onSubmit={handleUpdate}>
                        <div className="container">
                            <div className="row d-flex justify-content-center">
                                <div className="col-sm-4 col-md-6 d-flex justify-content-center">
                                    <input
                                        type="text"
                                        className="rounded-pill"
                                        style={{ fontSize: '1.25rem' }}
                                        value={newCollectionName}
                                        placeholder={currentCollection}
                                        onChange={(e) => setNewCollectionName(e.target.value)}
                                        required
                                    />
                                </div>
                                <div className="col-sm-4 col-md-6 d-flex justify-content-center">
                                    <div className="row d-flex justify-content-center">        
                                        <div className="col-6 d-flex justify-content-center">  
                                            <button 
                                                className="rounded-pill"
                                                style={{ fontSize: '1.25rem' }}
                                                type="submit"><i className="bi bi-check-lg"></i>
                                            </button>
                                        </div>
                                        <div className="col-6 d-flex justify-content-center">
                                            <button
                                                type="button"
                                                onClick={() => setIsModalOpen(false)}
                                                className="rounded-pill"
                                                style={{ fontSize: '1.25rem' }}
                                            ><i className="bi bi-x-lg"></i></button>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </form>
                </div>
            </ReactModal>
            <div style={{ marginTop: '20px', textAlign: 'center' }}>
                <p>Version: {version}</p>
                <p>Heartbeat: {heartbeat}</p>
            </div>
        </div>
    );
};

export default CollectionsManagement;