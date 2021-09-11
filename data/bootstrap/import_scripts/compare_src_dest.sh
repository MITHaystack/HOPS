#
# This is the common comparison engine, sourced by the other scripts
# It loops over an array variable source_list and does the hash check
# and as necessary (if CHKSUM=0) an import cp or rm
#
[ -n "$src_dir" -a -d "$src_dir" ] ||
    { echo no src_dir $src_dir; return 1; }
[ -n "$dest_dir" -a -d "$dest_dir" ] ||
    { echo no dest_dir $dest_dir; return 1; }
[ -n "$CHKSUM" ] && [ "$CHKSUM" -eq 0 -o "$CHKSUM" -eq 1 ] ||
    { echo CHKSUM must be 0 or 1, $CHKSUM; return 1; }

retval=0
for i in "${source_list[@]}"
do
    # compute hashes
    [ -f "${src_dir}/${i}" ] &&
        SOURCE_HASH=$( md5sum "${src_dir}/${i}" |\
            awk '{print $1}' | tr -d '\n') &&
        SOURCE_HASH="${SOURCE_HASH%% *}" ||
        SOURCE_HASH='no-source'
    [ -f "${dest_dir}/${i}" ] &&
        DEST_HASH=$( md5sum "${dest_dir}/${i}" |\
            awk '{print $1}' | tr -d '\n') &&
        DEST_HASH="${DEST_HASH%% *}" ||
        DEST_HASH='no-dest'
    # no action case first
    [ "${SOURCE_HASH}" = "${DEST_HASH}" ] &&
        continue

    # script errors next
    [ "${SOURCE_HASH}" = 'no-source' ] &&
        echo source "${src_dir}/${i}" missing--fix "'$part'" &&
        retval=$((retval + 1)) && {
            # fix if enabled:
            [ "$CHKSUM" -eq 0 ] &&
            echo removing "${dest_dir}/${i}" &&
            rm -f "${dest_dir}/${i}" || true ; } &&
        continue
    [ "${DEST_HASH}" = 'no-dest' ] &&
        echo missing dest "${dest_dir}/${i}" missing--fix "'$part'" &&
        retval=$((retval + 1)) && {
            # fix if enabled:
            [ "$CHKSUM" -eq 0 ] &&
            echo importing "${dest_dir}/${i}" &&
            cp -p "${src_dir}/${i}" "${dest_dir}/${i}" || true ; } &&
        continue

    # finally, hashes differ and we have permission
    [ $CHKSUM -eq 0 ] &&
        echo importing ${i} for hash difference
        cp -p "${src_dir}/${i}" "${dest_dir}/${i}"
done

return $retval
#
# eof
#
