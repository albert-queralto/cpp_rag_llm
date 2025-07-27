# Create the data directory if it doesn't exist
mkdir -p '/root/home/data'

# Check if the database file exists; if not, initialize it
if ! [ -f '/root/home/data/database.db' ] ;
then
    echo "Inexistent Database. Initializing..."
    sqlite3 -batch '/root/home/data/database.db' < '/root/home/backend/db/sql/users.sql'
    chmod 664 '/root/home/data/database.db'
fi

# Build the C++ backend application if the executable doesn't exist
# if ! [ -f '/root/home/backend/build/llm_backend' ] ;
# then
#     echo "Backend executable not found. Building the application..."
#     mkdir -p '/root/home/backend/build'
#     cd '/root/home/backend/build' || exit
#     cmake ..
#     make
# fi
# Always build the C++ backend application
echo "Building the backend application..."
mkdir -p '/root/home/backend/build'
cd '/root/home/backend/build' || exit
cmake ..
make

# Run the C++ backend application
echo "Starting C++ backend application..."
'/root/home/backend/build/llm_backend'

exit