import { useState, useEffect, useRef } from "react";
import Select from 'react-select';
import { ToastContainer, toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import { Tooltip } from 'react-tooltip';

function FileForm() {
    const [files, setFiles] = useState([]);
    const [collections, setCollections] = useState([]);
    const [selectedCollection, setSelectedCollection] = useState('');
    const [documents, setDocuments] = useState([]);
    const fileInputRef = useRef(null);

    useEffect(() => {
        fetchCollections();
    }, []);

    useEffect(() => {
        if (selectedCollection) {
            fetchDocuments(selectedCollection);
        }
    }, [selectedCollection]);

    const fetchCollections = async () => {
        try {
            const response = await fetch('http://localhost:4000/api/collections');
            if (response.ok) {
                const data = await response.json();
                setCollections(data);
            } else {
                console.error('HTTP Error:', response.status, response.statusText);
            }
        } catch (error) {
            console.error('Failed to fetch:', error.message);
        }
    };

    const fetchDocuments = async (collection) => {
        try {
            const response = await fetch(`http://localhost:4000/api/get_document_filenames/${encodeURIComponent(collection)}`);
            if (response.ok) {
                const data = await response.json();
                console.log(data);
                setDocuments(data);
            } else {
                console.error('Failed to fetch:', response.status, response.statusText);
                setDocuments({ filenames: [] });
            }
        } catch (error) {
            console.error('Failed to fetch:', error.message);
            setDocuments({ filenames: [] });
        }
    };

    const handleDragOver = (event) => {
        event.preventDefault();
        event.stopPropagation();
    };

    const handleDragLeave = (event) => {
        event.preventDefault();
        event.stopPropagation();
    };

    const handleDrop = (event) => {
        event.preventDefault();
        event.stopPropagation();
        const droppedFiles = Array.from(event.dataTransfer.files);
        if (droppedFiles.length > 0) {
            setFiles(prevFiles => [...prevFiles, ...droppedFiles]);
        }
    };

    const handleFileChange = (event) => {
        const selectedFiles = Array.from(event.target.files);
        if (selectedFiles.length > 0) {
            setFiles(prevFiles => [...prevFiles, ...selectedFiles]);
        }
    };

    // --- Modified handleSubmit for single file upload endpoint ---
    const handleSubmit = async (event) => {
        event.preventDefault();
        if (files.length === 0) {
            toast.error("No files selected.");
            return;
        }
        if (!selectedCollection) {
            toast.error("No collection selected.");
            return;
        }

        for (const file of files) {
            try {
                const endpoint = `http://localhost:4000/api/upload/${encodeURIComponent(selectedCollection)}`;
                const response = await fetch(endpoint, {
                    method: "POST",
                    headers: {
                        "X-Filename": file.name
                    },
                    body: file
                });

                if (response.ok) {
                    toast.success(`File "${file.name}" uploaded successfully!`);
                } else {
                    toast.error(`Failed to upload "${file.name}".`);
                }
            } catch (error) {
                console.error(error);
                toast.error(`An error occurred while uploading "${file.name}".`);
            }
        }
        setFiles([]);
        fetchDocuments(selectedCollection);
    };

    const collectionOptions = collections.map((collection) => ({
        value: collection.name,
        label: collection.name,
    }));

    const handleSelectChange = (selectedOption) => {
        setSelectedCollection(selectedOption.value);
    };

    const customSelectStyles = {
        control: (provided) => ({
            ...provided,
            border: '1px solid #0e2f48',
            borderRadius: '0.5rem',
            fontSize: '1.25rem',
            fontWeight: selectedCollection ? 'bold' : 'normal',
        }),
        option: (provided, state) => ({
            ...provided,
            color: state.isSelected ? 'white' : 'black',
            backgroundColor: state.isSelected ? '#7FAD3D' : 'white',
            fontWeight: state.isSelected ? 'bold' : 'normal',
            '&:hover': {
                backgroundColor: '#0e2f48',
                color: 'white',
            },
            fontSize: '1.25rem',
        }),
    };

    const deleteDocument = async (filename) => {
        try {
            const response = await fetch(`http://localhost:4000/api/delete_documents_by_filename/${encodeURIComponent(selectedCollection)}/${encodeURIComponent(filename)}`, {
                method: 'DELETE',
            });
            if (response.ok) {
                toast.success("Document deleted successfully!");
                fetchDocuments(selectedCollection);
            } else {
                toast.error("Failed to delete document.");
            }
        } catch (error) {
            console.error('Error deleting document:', error);
            toast.error("Error deleting document.");
        }
    };
        
    return (
        <>
            <header className="chat-header">
                <div style={{ flexGrow: 1, display: "flex", justifyContent: "center" }}>
                    File Management
                </div>
            </header>

            <div className="collections-selector">
                <div style={{ marginTop: '10px', marginBottom: '10px' }}>
                    <Select
                        options={collectionOptions}
                        value={collectionOptions.find(option => option.value === selectedCollection)}
                        onChange={handleSelectChange}
                        placeholder="Select a collection..."
                        styles={customSelectStyles}
                    />
                </div>
            </div>
            
            <form onSubmit={handleSubmit}>
                <div className="container d-flex flex-column">
                    <div className="row justify-content-center">
                        <div
                            className="col-md-8 d-flex justify-content-center"
                            onDragOver={handleDragOver}
                            onDragLeave={handleDragLeave}
                            onDrop={handleDrop}
                            onClick={() => fileInputRef.current.click()}
                            style={{
                                width: '400px',
                                height: '150px',
                                border: '2px dashed #ccc',
                                display: 'flex',
                                alignItems: 'center',
                                justifyContent: 'center',
                                marginTop: '10px',
                                marginBottom: '10px',
                                cursor: 'pointer'
                            }}
                        >
                            Drag and drop files here or click to select
                        </div>
                        <input
                            type="file"
                            ref={fileInputRef}
                            style={{ display: 'none' }}
                            onChange={handleFileChange}
                            multiple
                        />
                    </div>
                    {files.length > 0 && (
                        <div className="row justify-content-center">
                            <div className="col-md-8 d-flex justify-content-center">
                                <ul>
                                    {files.map((file, index) => (
                                        <li key={index}>{file.name}</li>
                                    ))}
                                </ul>
                            </div>
                        </div>
                    )}
                    <div className="row justify-content-center">
                        <div className="col-md-4 d-flex justify-content-center">
                            <button
                                className="block py-3 px-4 w-full border outline-none focus:ring focus:outline-none rounded-pill" 
                                style={{ fontSize: '1.25rem' }}
                                type="submit"
                            >Upload documents</button>
                        </div>
                    </div>
                </div>
            </form>

            <div className="documents-table">
                {documents.filenames && documents.filenames.length > 0 ? (
                    <table style={{ borderCollapse: 'separate', borderSpacing: '20px 20px', textAlign: 'left', fontSize: '1.25rem' }}>
                        <thead>
                            <tr>
                                <th>Element</th>
                                <th>Actions</th>
                            </tr>
                        </thead>
                        <tbody>
                        {documents.filenames.map((filename, index) => (
                            <tr key={index}>
                                <td>{filename}</td>
                                <td>
                                    <button 
                                        id="delete-tooltip"
                                        onClick={() => deleteDocument(filename)}
                                        className="rounded-pill"
                                        style={{ fontSize: '1.25rem' }}
                                    ><i className="bi bi-x-lg"></i></button>
                                    <Tooltip anchorSelect="#delete-tooltip" content="Delete Document" />
                                </td>
                            </tr>
                        ))}
                        </tbody>
                    </table>
                ) : (
                    <p>No documents found.</p>
                )}
            </div>

        <ToastContainer />
        </>
    )
}

export default FileForm;